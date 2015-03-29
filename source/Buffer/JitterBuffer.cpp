#include "JitterBuffer.h"
#include "../SynchronizationHelper.h"

#include <map>
#include <cmath>
#include <stdio.h>

JitterBuffer::JitterBuffer(int capacity, int elementSize, int delay, int interval) : Heap(capacity,elementSize)
{
    this->lastIndex   = 0;
    this->delay       = delay;
    this->interval    = interval;
    this->canGet      = CreateEvent(NULL,TRUE,FALSE,NULL);
    this->access      = CreateMutex(NULL, FALSE, NULL);
    this->notFull     = CreateSemaphore(NULL,capacity,capacity,NULL);
    this->notEmpty    = CreateSemaphore(NULL,0,capacity,NULL);
}

/**
 * puts the passed data into the jitter buffer if the index of the last element
 *   removed is smaller than the index of the element being inserted (i.e. this
 *   element is being inserted too late, since elements that should be consumed
 *   after it have already been consumed).
 *
 * @function   JitterBuffer::insert
 *
 * @date       2015-03-19
 *
 * @revision   none
 *
 * @designer   Eric Tsang
 *
 * @programmer Eric Tsang
 *
 * @note       none
 *
 * @signature  int JitterBuffer::insert(int index, void* src)
 *
 * @param      index index of the element being inserted.
 * @param      src pointer to data to be copied into the element.
 *
 * @return     0 upon success, 1 upon rejection.
 */
int JitterBuffer::put(int index, void* src)
{
    int ret;

    // acquire synchronization objects
    WaitForSingleObject(notFull,INFINITE);
    WaitForSingleObject(access,INFINITE);

    if(index > lastIndex)
    {
        ret = 0;

        // if this is the first element after the buffer is empty,
        // delay...
        if(Heap::size() == 0)
        {
            delayedSetEvent(canGet,delay);
        }

        // put the new element into the heap
        Heap::insert(index,src);
    }
    else
    {
        ret = 1;
    }

    // release synchronization objects
    ReleaseMutex(access);
    ReleaseSemaphore(notEmpty,1,NULL);

    return ret;
}

void JitterBuffer::get(void* dest)
{
    // acquire synchronization objects
    WaitForSingleObject(notEmpty,INFINITE);
    WaitForSingleObject(access,INFINITE);
    WaitForSingleObject(canGet,INFINITE);

    // copy data from root to destination
    Heap::remove(&lastIndex,dest);

    // reset the canGet event, and set it after
    // interval
    ResetEvent(canGet);
    delayedSetEvent(canGet,interval);

    // release synchronization objects
    ReleaseMutex(access);
    ReleaseSemaphore(notFull,1,NULL);
}
