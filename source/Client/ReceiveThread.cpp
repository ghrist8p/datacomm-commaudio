#include "ReceiveThread.h"
#include "../Buffer/MessageQueue.h"
#include "../Buffer/JitterBuffer.h"
#include "PlaybackTrackerPanel.h"
#include "ButtonPanel.h"
#include "FileListItem.h"
#include "ConnectionWindow.h"
#include "../Buffer/MessageQueue.h"
#include "../Buffer/JitterBuffer.h"
#include "ReceiveThread.h"
#include "VoiceBufferer.h"
#include "MicReader.h"
#include "PlayWave.h"
#include "../protocol.h"

// static function forward declarations

static int startRoutine(HANDLE* thread, HANDLE stopEvent,
    LPTHREAD_START_ROUTINE routine, void* params);
static int stopRoutine(HANDLE* thread, HANDLE stopEvent);
JitterBuffer* getJitterBuffer(unsigned long srcAddr);

// receive thread implementation

ReceiveThread::ReceiveThread(JitterBuffer* musicJitterBuffer, MessageQueue* sockMsgQueue)
{
    this->sockMsgQueue      = sockMsgQueue;
    this->musicJitterBuffer = musicJitterBuffer;
    this->thread            = INVALID_HANDLE_VALUE;
    this->threadStopEv      = CreateEvent(NULL,TRUE,FALSE,NULL);
}

ReceiveThread::~ReceiveThread()
{
    stop();
}

void ReceiveThread::start()
{
    startRoutine(&thread,threadStopEv,threadRoutine,this);
}

void ReceiveThread::stop()
{
    stopRoutine(&thread,threadStopEv);
}

DWORD WINAPI ReceiveThread::threadRoutine(void* params)
{
    #ifdef DEBUG
    OutputDebugStream("Thread started...\n");
    #endif

    // parse thread parameters
    ReceiveThread* dis = (ReceiveThread*) params;

    // perform the thread routine
    int breakLoop = FALSE;
    while(!breakLoop)
    {
        HANDLE handles[] = {
            dis->threadStopEv,
            dis->sockMsgQueue->hasMessage
        };
        switch(WaitForMultipleObjects(2,handles,FALSE,INFINITE))
        {
        case WAIT_OBJECT_0+0:   // stop event triggered
            breakLoop = TRUE;
            break;
        case WAIT_OBJECT_0+1:   // message queue has message
            ReceiveThread::handleMsgqMsg(dis);
            break;
        default:
            int err = GetLastError();
            OutputDebugString(L"ReceiveThread::_threadRoutine WaitForMultipleObjects");
            break;
        }
    }

    // return...
    #ifdef DEBUG
    OutputDebugStream("Thread stopped...\n");
    #endif
    return 0;
}

void ReceiveThread::handleMsgqMsg(ReceiveThread* dis)
{
    // allocate memory to hold message queue message
    int msgType;
    char* element = (char*) malloc(dis->sockMsgQueue->elementSize);

    // get the message queue message
    dis->sockMsgQueue->dequeue(&msgType,element);

    // process the message queue message according to its type
    switch(msgType)
    {
    case MUSICSTREAM:
    {
        LocalDataPacket* packet = (LocalDataPacket*) element;
        dis->musicJitterBuffer->put(packet->index,packet->data);
        break;
    }
    case MICSTREAM:
    {
        LocalDataPacket* packet = (LocalDataPacket*) element;
        JitterBuffer* jb = getJitterBuffer(packet->srcAddr);
        jb->put(packet->index,packet->data);
        MessageQueue* queue = new MessageQueue(1500,DATA_LEN);
        VoiceBufferer* voiceBufferer = new VoiceBufferer(queue,jb);
        voiceBufferer->start();
        PlayWave* playWave = new PlayWave(50,queue);
        playWave->startPlaying(DATA_LEN, MIC_BITS_PER_SAMPLE, NUM_MIC_CHANNELS);
        break;
    }
    default:
        fprintf(stderr,"WARNING: received unknown message type: %d\n",msgType);
        break;
    }

    // deallocate the allocated memory
    free(element);
}

// static function implementations

/**
 * returns a jitter buffer used to store voice data from a specific source
 *   address; instantiates a new jitter buffer if needed.
 *
 * @date     2015-04-05T19:51:23-0800
 *
 * @author   Eric Tsang
 *
 * @param    srcAddr   source address used to identify which jitter buffer to
 *   return.
 *
 * @return   the jitter buffer used to store the voice data from the passed
 *   source address
 */
JitterBuffer* getJitterBuffer(unsigned long srcAddr)
{
    // maps source IP addresses to jitter buffers
    static std::map<unsigned long,JitterBuffer*> voiceJitterBuffers;

    // if the jitter buffer doesn't exist make one, put it into the map
    if(voiceJitterBuffers.find(srcAddr) == voiceJitterBuffers.end())
    {
        voiceJitterBuffers[srcAddr] = new JitterBuffer(5000,100,DATA_LEN,50,50);;
    }

    // return the jitter buffer for the passed srcAddr
    return voiceJitterBuffers[srcAddr];
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
