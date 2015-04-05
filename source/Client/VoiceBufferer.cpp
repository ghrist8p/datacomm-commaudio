#include "VoiceBufferer.h"
#include "../Buffer/JitterBuffer.h"
#include "../Buffer/MessageQueue.h"
#include "Sockets.h"

// static function forward declarations

static int startRoutine(HANDLE* thread, HANDLE stopEvent,
    LPTHREAD_START_ROUTINE routine, void* params);
static int stopRoutine(HANDLE* thread, HANDLE stopEvent);

VoiceBufferer::VoiceBufferer(
    MessageQueue* speakerQueue,
    JitterBuffer* voiceJitterBuffer)
{
    this->speakerQueue      = speakerQueue;
    this->voiceJitterBuffer = voiceJitterBuffer;
    this->thread            = INVALID_HANDLE_VALUE;
    this->threadStopEv      = CreateEvent(NULL,TRUE,FALSE,NULL);
}

VoiceBufferer::~VoiceBufferer()
{
    stop();
}

void VoiceBufferer::start()
{
    startRoutine(&thread,threadStopEv,_threadRoutine,this);
}

void VoiceBufferer::stop()
{
    stopRoutine(&thread,threadStopEv);
}

DWORD WINAPI VoiceBufferer::_threadRoutine(void* params)
{
    #ifdef DEBUG
    OutputDebugStream("Thread started...\n");
    #endif

    // parse thread parameters
    VoiceBufferer* dis = (VoiceBufferer*) params;

    // allocate data to hold parameters
    char* element = (char*) malloc(dis->voiceJitterBuffer->getElementSize());

    // perform the thread routine
    int breakLoop = FALSE;
    while(!breakLoop)
    {
        HANDLE handles[] = {
            dis->threadStopEv,
            dis->voiceJitterBuffer->canGet
        };
        switch(WaitForMultipleObjects(2,handles,FALSE,INFINITE))
        {
        case WAIT_OBJECT_0+0:   // stop event triggered
            breakLoop = TRUE;
            break;
        case WAIT_OBJECT_0+1:   // jitter buffer has data
        {
            dis->voiceJitterBuffer->get(element);
            dis->speakerQueue->enqueue(1,element);
            break;
        }
        default:
            OutputDebugString(L"VoiceBufferer::_threadRoutine WaitForMultipleObjects");
            break;
        }
    }

    // deallocate data
    free(element);

    // return...
    #ifdef DEBUG
    OutputDebugStream("Thread stopped...\n");
    #endif
    return 0;
}

// static function implementations

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

