#ifndef _PLAY_WAVE_CPP_
#define _PLAY_WAVE_CPP_

#include "PlayWave.h"
#include "../common.h"
#include "../handlerHelper.h"
#include "../Buffer/MessageQueue.h"

#include <stdio.h>

#pragma warning(disable:4996)
#pragma comment(lib,"winmm.lib")

#define SAMPLE_RATE 44100
#define BITS_PER_SAMPLE 16
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
 * used as thread parameters for the freeAudioPacketOnceUsedRoutine.
 */
struct FapouParams
{
	PlayWave* dis;
	WAVEHDR* audioPacket;
};

typedef struct FapouParams FapouParams;

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
	this->playThreadStopEv = CreateEvent(NULL,TRUE,FALSE,NULL);
	this->lastAudioPacket = 0;
	this->lastAudioPacketAccess = CreateMutex(NULL, FALSE, NULL);
	this->canEnqueue = CreateSemaphore(NULL,capacity,capacity,NULL);
	this->canDequeue = CreateSemaphore(NULL,0,capacity,NULL);
}

PlayWave::~PlayWave()
{
	stopPlaying();
	closeDevice();
}

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
	return ret;
}

int PlayWave::stopPlaying()
{
	printf("PlayWave::stopPlaying called\n");
	stopRoutine(&playThread,playThreadStopEv);
	while(lastAudioPacket != 0);
	printf("PlayWave::stopPlaying returns\n");
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

void PlayWave::enqueueAudioData(char* data, int length)
{
	// allocate, and set header data
	WAVEHDR* audioPacket = (WAVEHDR*) malloc(sizeof(*audioPacket));

	// prepare the current audio packet
	memset(audioPacket,0,sizeof(*audioPacket));
	audioPacket->dwUser         = 0;
	audioPacket->lpData         = (char*) malloc(length);
	audioPacket->dwBufferLength = length;

	// copy the data from the user space to our own
	memcpy(audioPacket->lpData,data,length);

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
		freeAudioPacketOnceUsed(this,audioPacket);
	}
	lastAudioPacket = audioPacket;

	// release synchronization objects
	ReleaseMutex(lastAudioPacketAccess);
	ReleaseSemaphore(canDequeue,1,NULL);

	// prepare the header.
	waveOutPrepareHeader(speakers,audioPacket,sizeof(*audioPacket));
	waveOutWrite(speakers,audioPacket,sizeof(*audioPacket));
}

DWORD WINAPI PlayWave::playRoutine(void* params)
{
	printf("Thread started...\n");

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

	// return...
	printf("Thread stopped...\n");
	return 0;
}

void PlayWave::handleMsgqMsg()
{
	// allocate memory to hold message queue message
	int msgType;
	char* element = (char*) malloc(msgq->elementSize);

	// get the message queue message
	msgq->dequeue((int*)&msgType,element);

	// process the message queue message according to its type
	enqueueAudioData(element,msgq->elementSize);
}

/////////////////////////////////////
// static function implementations //
/////////////////////////////////////

void PlayWave::freeAudioPacketOnceUsed(PlayWave* dis, WAVEHDR* audioPacket)
{
	// prepare thread parameters
	FapouParams* params = (FapouParams*) malloc(sizeof(*params));
	params->dis = dis;
	params->audioPacket = audioPacket;

	// start the thread
	DWORD useless;
	CreateThread(0,0,freeAudioPacketOnceUsedRoutine,params,0,&useless);
}

DWORD WINAPI PlayWave::freeAudioPacketOnceUsedRoutine(void* params)
{
	printf("cleanup thread started\n");
	// parse thread parameters
	FapouParams* p = (FapouParams*) params;

	while(p->audioPacket != 0)
	{
		// wait for the "header finished being used" flag to be set
		while(!(p->audioPacket->dwFlags&WHDR_DONE))
		{
			printf("cleanup thread waiting\n");
			Sleep(CHECK_FOR_FREEING_INTERVAL);
		}

		// acquire synchronization objects
		WaitForSingleObject(p->dis->canDequeue,INFINITE);
		WaitForSingleObject(p->dis->lastAudioPacketAccess,INFINITE);

		// save where the next packet to deallocate is
		WAVEHDR* next = (WAVEHDR*) p->audioPacket->dwUser;

		// unprepare the audio header, free the audio header and free the
		// payload
		printf("cleanup thread freeing packet %p\n",p->audioPacket);
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
	printf("cleanup thread stopped\n");
	free(p);
	return 0;
}

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
	WaitForSingleObject(*thread,INFINITE);

	// invalidate thread handle, so we know it's terminated
	*thread = INVALID_HANDLE_VALUE;
	return 0;
}

///////////////
// main test //
///////////////

#define BUFSIZE 60

DWORD WINAPI stopAndStart(void* params)
{
	PlayWave* play = (PlayWave*) params;

	Sleep(5000);
	play->stopPlaying();
	Sleep(1000);
	play->startPlaying(SAMPLE_RATE,BITS_PER_SAMPLE,CHANNELS);

    return 0;
}

int main(void)
{
	MessageQueue msgq(10,BUFSIZE);
	PlayWave play(5000,&msgq);
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
