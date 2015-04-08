#ifndef _PLAY_WAVE_CPP_
#define _PLAY_WAVE_CPP_

#define DEBUG
//#define RUN_TEST

#include "PlayWave.h"
#include "../common.h"
#include "../handlerHelper.h"
#include "../Buffer/MessageQueue.h"

#include <stdio.h>

#pragma warning(disable:4996)
#pragma comment(lib,"winmm.lib")

#define SAMPLE_RATE 44100
#define BITS_PER_SAMPLE 8
#define CHANNELS 2
#define BITS_PER_BYTE 8
#define MILLISEC_PER_SEC 1000

/**
 * interval of time to wait between checks to free allocated audio packets to
 *   play.
 */
#define CHECK_FOR_FREEING_INTERVAL 100

using namespace std;

////////////////////////////
// structure declarations //
////////////////////////////

/**
 * used as thread parameters for the cleanupRoutine.
 */
struct CrParams
{
	PlayWave* dis;
	WAVEHDR* audioPacket;
};

typedef struct CrParams CrParams;

/////////////////////////////////////////
// static function forward declaration //
/////////////////////////////////////////
static int startRoutine(HANDLE* thread, HANDLE stopEvent,
	LPTHREAD_START_ROUTINE routine, void* params);
static int stopRoutine(HANDLE* thread, HANDLE stopEvent);

/////////////////////////////
// PlayWave implementation //
/////////////////////////////

/**
 * constructor for the {PlayWave} class. it initializes the instance variables,
 *   and audio settings to default settings.
 *
 * @date     2015-04-02T18:28:46-0800
 *
 * @author   Eric Tsang
 */
PlayWave::PlayWave(int capacity, MessageQueue* msgq)
{
	this->msgq = msgq;
	this->speakers = 0;
	this->playThread = INVALID_HANDLE_VALUE;
	this->cleanupThread = INVALID_HANDLE_VALUE;
	this->playThreadStopEv = CreateEvent(NULL,TRUE,FALSE,NULL);
	this->cleanupThreadEv = CreateEvent(NULL,TRUE,FALSE,NULL);
	this->lastAudioPacket = 0;
	this->lastAudioPacketAccess = CreateMutex(NULL, FALSE, NULL);
	this->canEnqueue = CreateSemaphore(NULL,capacity,capacity,NULL);
	this->canDequeue = CreateSemaphore(NULL,0,capacity,NULL);
}

/**
 * destructor for the audio device. when destructed, the play thread is stopped,
 *   and the class's handle to the audio device is invalidated.
 *
 * @date     2015-04-03T10:42:59-0800
 *
 * @author   Eric Tsang
 */
PlayWave::~PlayWave()
{
	stopPlaying();
	closeDevice();
}

/**
 * obtains a handle to the audio device, and starts the play thread, if there
 *   are no problems.
 *
 * @date     2015-04-03T10:44:30-0800
 *
 * @author   Eric Tsang
 *
 * @param    samplesPerSecond   the number of samples per second the audio device should play
 * @param    bitsPerSample   the number of bits used per sample for the passed PCM data
 * @param    numChannels   the number of channels there are in the passed PCM data.
 *
 * @return   see openDevice
 */
int PlayWave::startPlaying(
	int samplesPerSecond,
	int bitsPerSample,
	int numChannels)
{
	int ret = openDevice(samplesPerSecond,bitsPerSample,numChannels);
	if(ret == MMSYSERR_NOERROR)
	{
		startRoutine(&playThread,playThreadStopEv,playRoutine,this);
	}
	else
	{
		#ifdef DEBUG
		printf("failed to get the device: %d\n",GetLastError());
		#endif
	}
	return ret;
}

/**
 * stops the playing thread, then waits for the cleanup thread to stop as well,
 *   before closing the device and returning.
 *
 * @date     2015-04-03T10:49:13-0800
 *
 * @author   Eric Tsang
 *
 * @return   see closeDevice()
 */
int PlayWave::stopPlaying()
{
	#ifdef DEBUG
	printf("PlayWave::stopPlaying called\n");
	#endif
	stopRoutine(&playThread,playThreadStopEv);
    waveOutReset(speakers);
	stopRoutine(&cleanupThread,cleanupThreadEv);

	#ifdef DEBUG
	printf("PlayWave::stopPlaying returns\n");
	#endif
	return closeDevice();
}


/**
 * sets the internal settings structure of this instance used to decode and play
 *   passed PCM data.
 *
 * @date     2015-04-02T18:24:49-0800
 *
 * @author   Eric Tsang
 *
 * @param    settings
 *
 * pointer to an {AudioSettings} structure that contains settings describing how
 *   PCM data passed to this instance to play is decoded and played.
 *
 * the data is copied from the passed structure into the object's internal
 *   members, so it can be safely deallocated after this function returns.
 *
 * @return   this function can return the following values:
 *
 * MMSYSERR_NOERROR successfully obtained the audio device.
 *
 * MMSYSERR_ALLOCATED; specified resource is already allocated.
 *
 * MMSYSERR_BADDEVICEID; specified device identifier is out of range.
 *
 * MMSYSERR_NODRIVER; no device driver is present.
 *
 * MMSYSERR_NOMEM; unable to allocate or lock memory.
 *
 * WAVERR_BADFORMAT; attempted to open with an unsupported waveform-audio
 *   format.
 *
 * WAVERR_SYNC; the device is synchronous but waveoutopen was called without
 *   using the wave_allowsync flag.
 */
int PlayWave::openDevice(
	int samplesPerSecond,
	int bitsPerSample,
	int numChannels)
{
	// set settings
	wfx.nSamplesPerSec  = samplesPerSecond;
	wfx.wBitsPerSample  = bitsPerSample;
	wfx.nChannels       = numChannels;
	wfx.cbSize          = 0;
	wfx.wFormatTag      = WAVE_FORMAT_PCM;
	wfx.nBlockAlign     = (wfx.wBitsPerSample >> 3) * wfx.nChannels;
	wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;

	// open the audio device
	int ret = waveOutOpen(&speakers,WAVE_MAPPER,&wfx,0,0,CALLBACK_NULL);

	// if there is an error opening the device, set our speakers to 0
	if(ret != MMSYSERR_NOERROR)
	{
		perror("unable to open WAVE_MAPPER device\n");
		speakers = 0;
	}

	// return result of trying to get a device
	return ret;
}

/**
 * tries to close the handle to the audio device, and returns various return
 *   codes depending on the outcome of the operation.
 *
 * @date     2015-04-02T18:55:55-0800
 *
 * @author   Eric Tsang
 *
 * @return   this function can return the following values:
 *
 * MMSYSERR_NOERROR; successfully closed the audio device handle.
 *
 * MMSYSERR_INVALHANDLE; specified device handle is invalid.
 *
 * MMSYSERR_NODRIVER; no device driver is present.
 *
 * MMSYSERR_NOMEM; unable to allocate or lock memory.
 *
 * WAVERR_STILLPLAYING; there are still buffers in the queue.
 */
int PlayWave::closeDevice()
{
	// try to close the device
	int ret = waveOutClose(speakers);

	// if success, set our device handle to 0
	if(ret == MMSYSERR_NOERROR)
	{
		speakers = 0;
	}

	// return the result of the operation
	return ret;
}

/**
 * the play routine reads audio data from the message queue into the speaker's
 *   output buffers as quickly as possible, until the thread is stopped.
 *
 * @date     2015-04-03T11:12:51-0800
 *
 * @author   Eric Tsang
 *
 * @param    params   pointer to the calling PlayWave instance.
 *
 * @return   exit code
 */
DWORD WINAPI PlayWave::playRoutine(void* params)
{
	#ifdef DEBUG
	printf("Thread started...\n");
	#endif

	// parse thread parameters
	PlayWave* dis = (PlayWave*) params;

	// perform the thread routine
	int breakLoop = FALSE;
	while(!breakLoop)
	{
		HANDLE handles[] = {
			dis->playThreadStopEv,
			dis->msgq->hasMessage
		};
		switch(WaitForMultipleObjects(2,handles,FALSE,INFINITE))
		{
		case WAIT_OBJECT_0+0:   // stop event triggered
			breakLoop = TRUE;
			break;
		case WAIT_OBJECT_0+1:   // message queue has message
			dis->handleMsgqMsg();
			break;
		default:
			fatalError("PlayWave::playRoutine WaitForMultipleObjects");
			break;
		}
	}


	#ifdef DEBUG
	// return...
	printf("Thread stopped...\n");
	#endif
	return 0;
}

/**
 * invoked to handle data from the message queue. the type of data is ignored.
 *   all data in the message queue is assumed to be PCM data that should be
 *   played.
 *
 * reads data from the message queue, and enqueues it to the speaker's output
 *   buffer, waiting to be played.
 *
 * this function may block if the speaker's audio buffer is full.
 *
 * @date     2015-04-03T11:17:36-0800
 *
 * @author   Eric Tsang
 */
void PlayWave::handleMsgqMsg()
{
	// allocate, and set header data
	WAVEHDR* audioPacket = (WAVEHDR*) malloc(sizeof(*audioPacket));

	// prepare the current audio packet
	memset(audioPacket,0,sizeof(*audioPacket));
	audioPacket->dwUser         = 0;
	audioPacket->lpData         = (char*) malloc(msgq->elementSize);
	audioPacket->dwBufferLength = msgq->elementSize;

	// copy the audio data from message queue to our own buffer
	int useless;
	msgq->dequeue((int*)&useless,audioPacket->lpData);

	// prepare the header.
	waveOutPrepareHeader(speakers,audioPacket,sizeof(*audioPacket));
	waveOutWrite(speakers,audioPacket,sizeof(*audioPacket));

	// acquire synchronization objects
	WaitForSingleObject(canEnqueue,INFINITE);
	WaitForSingleObject(lastAudioPacketAccess,INFINITE);

	// link the previous audio packet, if it has not been deallocated yet, to
	// the current packet. if it has been deallocated already, then start
	// another thread used to deallocate them
	if(lastAudioPacket != 0)
	{
		lastAudioPacket->dwUser = (DWORD_PTR) audioPacket;
	}
	else
	{
		startCleanupRoutine(this,audioPacket);
	}
	lastAudioPacket = audioPacket;

	// release synchronization objects
	ReleaseMutex(lastAudioPacketAccess);
	ReleaseSemaphore(canDequeue,1,NULL);
}

void PlayWave::startCleanupRoutine(PlayWave* dis, WAVEHDR* audioPacket)
{
	// prepare thread parameters
	CrParams* params = (CrParams*) malloc(sizeof(*params));
	params->dis = dis;
	params->audioPacket = audioPacket;

	// start the thread
	startRoutine(&dis->cleanupThread,dis->cleanupThreadEv,cleanupRoutine,params);
}

DWORD WINAPI PlayWave::cleanupRoutine(void* params)
{
	#ifdef DEBUG
	OutputDebugString(L"cleanup thread started\n");
	#endif
	// parse thread parameters
	CrParams* p = (CrParams*) params;

	while(p->audioPacket != 0)
	{
		// wait for the "header finished being used" flag to be set. the flag is
		// set once the audio device has finished reading it, and playing out
		// the speakers
		while(!(p->audioPacket->dwFlags&WHDR_DONE))
		{
			#ifdef DEBUG
			OutputDebugString(L"cleanup thread waiting\n");
			#endif
            //if(WaitForSingleObject(p->dis->cleanupThreadEv,CHECK_FOR_FREEING_INTERVAL) == WAIT_OBJECT_0)
            //{
            //    break;
            //}
			Sleep(CHECK_FOR_FREEING_INTERVAL);
		}

		// acquire synchronization objects
		WaitForSingleObject(p->dis->canDequeue,INFINITE);
		WaitForSingleObject(p->dis->lastAudioPacketAccess,INFINITE);

		// save where the next packet to deallocate is
		WAVEHDR* next = (WAVEHDR*) p->audioPacket->dwUser;

		// unprepare the audio header, free the audio header and free the
		// payload
		#ifdef DEBUG
        char output[256];
		wsprintf((LPWSTR)output,L"cleanup thread freeing packet %p\n",p->audioPacket);
        OutputDebugString((LPWSTR)output);
		#endif
		waveOutUnprepareHeader(p->dis->speakers,p->audioPacket,sizeof(WAVEHDR));
		free(p->audioPacket->lpData);
		free(p->audioPacket);

		// if the packet we deallocated is the class's "last audio packet", then
		// set the class's "last audio packet" to 0, indicating to it that it
		// has been deallocated
		if(p->dis->lastAudioPacket == p->audioPacket)
		{
			p->dis->lastAudioPacket = 0;
		}

		// assign the next packet packet to deallocate
		p->audioPacket = next;

		// release synchronization objects
		ReleaseMutex(p->dis->lastAudioPacketAccess);
		ReleaseSemaphore(p->dis->canEnqueue,1,NULL);
	}

	// free thread parameters & return
	#ifdef DEBUG
	OutputDebugString(L"cleanup thread stopped\n");
	#endif
	stopRoutine(&p->dis->cleanupThread,0);
	free(p);
	return 0;
}

/////////////////////////////////////
// static function implementations //
/////////////////////////////////////

int startRoutine(HANDLE* thread, HANDLE stopEvent,
	LPTHREAD_START_ROUTINE routine, void* params)
{
	// return immediately if the routine is already running
	if(*thread != INVALID_HANDLE_VALUE)
	{
		return 1;
	}

	// reset the stop event
	ResetEvent(stopEvent);

	// start the thread & return
	DWORD useless;
	*thread = CreateThread(0,0,routine,params,0,&useless);
	return (*thread == INVALID_HANDLE_VALUE);
}

int stopRoutine(HANDLE* thread, HANDLE stopEvent)
{
	// return immediately if the routine is already stopped
	if(*thread == INVALID_HANDLE_VALUE)
	{
		return 1;
	}

	// set the stop event to stop the thread
	SetEvent(stopEvent);
    if(GetCurrentThreadId() != GetThreadId(*thread))
    {
	    WaitForSingleObject(*thread,INFINITE);
    }

	// invalidate thread handle, so we know it's terminated
	*thread = INVALID_HANDLE_VALUE;
	return 0;
}

///////////////
// main test //
///////////////

#ifdef RUN_TEST

#define BUFSIZE 60

DWORD WINAPI stopAndStart(void* params)
{
	PlayWave* play = (PlayWave*) params;

	Sleep(5000);
	play->stopPlaying();
	Sleep(3000);
	play->startPlaying(SAMPLE_RATE,BITS_PER_SAMPLE,CHANNELS);

    return 0;
}

int main(void)
{
	MessageQueue msgq(10,BUFSIZE);
	PlayWave play(500,&msgq);
	DWORD useless;
	CreateThread(0,0,stopAndStart,&play,0,&useless);
	play.startPlaying(SAMPLE_RATE,BITS_PER_SAMPLE,CHANNELS);
	char sound[BUFSIZE];
	FILE* fp = fopen("C:\\Users\\Eric\\Downloads\\Egoist_-_Extra_Terrestrial_Biological_Entities.wav","rb");
	while(fread(sound,1,BUFSIZE,fp))
	{
		msgq.enqueue(0,sound);
	}
	play.stopPlaying();
}

#endif

#endif
