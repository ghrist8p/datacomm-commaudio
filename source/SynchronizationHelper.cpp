#include "SynchronizationHelper.h"

///////////////////////////
//  forward declarations //
///////////////////////////

static DWORD WINAPI delayedSetEventRoutine(LPVOID params);
static HANDLE access = CreateMutex(NULL, FALSE, NULL);;

struct DelayedSetEventParams
{
    HANDLE event;
    long milliseconds;
};

typedef struct DelayedSetEventParams DelayedSetEventParams;

//////////////////////////////
// function implementations //
//////////////////////////////

void delayedSetEvent(HANDLE event, long milliseconds)
{
    // obtain synchronization objects
    WaitForSingleObject(access,INFINITE);

    // prepare thread parameters
    DelayedSetEventParams* params;
    params = (DelayedSetEventParams*) malloc(sizeof(DelayedSetEventParams));
    params->event        = event;
    params->milliseconds = milliseconds;

    // create the thread
    DWORD useless;
    CreateThread(0,0,delayedSetEventRoutine,params,0,&useless);

    // release synchronization objects
    ReleaseMutex(access);
}

static DWORD WINAPI delayedSetEventRoutine(LPVOID params)
{
    // parse thread parameters
    DelayedSetEventParams* p = (DelayedSetEventParams*) params;

    // wait for the specified time, then set the event
    Sleep(p->milliseconds);
    SetEvent(p->event);

    // free the malloced parameters & return
    free(p);
    return 0;
}
