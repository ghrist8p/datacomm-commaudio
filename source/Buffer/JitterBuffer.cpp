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
 * @return     1 upon success, 0 upon rejection.
 */
int JitterBuffer::put(int index, void* src)
{
    // acquire synchronization objects
    WaitForSingleObject(notFull,INFINITE);
    WaitForSingleObject(access,INFINITE);

    if(index > lastIndex)
    {
        // if this is the first element after the buffer is empty,
        // delay...
        if(Heap::size() == 0)
        {
            delayedSetEvent(canGet,delay);
        }

        // put the new element into the heap
        Heap::insert(index,src);
    }

    // release synchronization objects
    ReleaseMutex(access);
    ReleaseSemaphore(notEmpty,1,NULL);

    return index > lastIndex;
}

/**
 * copies the next element from th {JitterBuffer} to the {dest} pointer. if no
 *   element was inserted for the element being removed, then the function
 *   returns 0; 1 otherwise.
 *
 * @function   JitterBuffer::get
 *
 * @date       2015-03-24
 *
 * @revision   none
 *
 * @designer   Eric Tsang
 *
 * @programmer Eric Tsang
 *
 * @note       none
 *
 * @signature  int JitterBuffer::get(void* dest)
 *
 * @param      dest pointer to copy element data into
 *
 * @return     1 if there was an inserted to remove from the JitterBuffer; 0
 *   otherwise.
 */
int JitterBuffer::get(void* dest)
{
    // acquire synchronization objects
    WaitForSingleObject(notEmpty,INFINITE);
    WaitForSingleObject(access,INFINITE);
    WaitForSingleObject(canGet,INFINITE);

    // copy data from root to destination
    int tempIndex;
    Heap::peek(&tempIndex,dest);

    if(++lastIndex == tempIndex)
    {
        Heap::remove();
    }

    // reset the canGet event, and set it after
    // interval
    ResetEvent(canGet);
    delayedSetEvent(canGet,interval);

    // release synchronization objects
    ReleaseMutex(access);
    ReleaseSemaphore(notFull,1,NULL);

    return lastIndex == tempIndex;
}
