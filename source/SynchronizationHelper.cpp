#include "SynchronizationHelper.h"

///////////////////////////
//  forward declarations //
///////////////////////////

static DWORD WINAPI delayedSetEventRoutine(LPVOID params);
static HANDLE memoryCopyAccess = CreateMutex(NULL, FALSE, NULL);;
static HANDLE delayedSetEventAccess = CreateMutex(NULL, FALSE, NULL);;

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
    WaitForSingleObject(delayedSetEventAccess,INFINITE);

    // prepare thread parameters
    DelayedSetEventParams* params;
    params = (DelayedSetEventParams*) malloc(sizeof(DelayedSetEventParams));
    params->event        = event;
    params->milliseconds = milliseconds;

    // create the thread
    DWORD useless;
    CreateThread(0,0,delayedSetEventRoutine,params,0,&useless);

    // release synchronization objects
    ReleaseMutex(delayedSetEventAccess);
}

void memoryCopy(void* dest, void* src, int len)
{
    // obtain synchronization objects
    WaitForSingleObject(memoryCopyAccess,INFINITE);

    // do the memcpy
    memcpy(dest,src,len);

    // release synchronization objects
    ReleaseMutex(memoryCopyAccess);
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
