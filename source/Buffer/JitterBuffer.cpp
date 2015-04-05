#include "JitterBuffer.h"
#include "../SynchronizationHelper.h"

#include <map>
#include <cmath>
#include <stdio.h>

/**
 * constructor for a {JitterBuffer} instance.
 *
 * @param capacity    maximum number of elements that the buffer can contain. if
 *   the buffer is at its maximum capacity, any attempt to add things to the
 *   buffer using the {JitterBuffer::put} method will block until there is more
 *   room in the {JitterBuffer}.
 * @param himark      minimum number of elements in the buffer before the buffer
 *   stops regulating the rate that elements can be removed from the buffer
 *   (using the {JitterBuffer::get} method).
 * @param elementSize the size of a single element in the JitterBuffer in bytes.
 * @param delay       milliseconds to wait after the first element is put in the
 *   JitterBuffer before it can be removed from the buffer. So, when the buffer
 *   is initially empty, and a new element is put into the buffer, any call to
 *   {JitterBuffer::get} will block for {delay} milliseconds before unblocking,
 *   and returning an element.
 * @param interval    interval in milliseconds between calls to
 *   {JitterBuffer::get} will block for; milliseconds a call to
 *   {JitterBuffer::get} will block for until it returns after each call to it.
 */
JitterBuffer::JitterBuffer(int capacity, int himark, int elementSize, int delay, int interval) : Heap(capacity,elementSize)
{
    this->lastIndex   = 0;
    this->delay       = delay;
    this->himark      = himark;
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
        ReleaseSemaphore(notEmpty,1,NULL);
    }
    else
    {
        ReleaseSemaphore(notFull,1,NULL);
    }

    // release synchronization objects
    ReleaseMutex(access);

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
    Heap::remove(&lastIndex,dest);
    ReleaseSemaphore(notFull,1,NULL);

    // reset the canGet event, and set it after
    // delay if we're out of data
    if(Heap::size() == 0)
    {
        ResetEvent(canGet);
        delayedSetEvent(canGet,delay);
    }

    // release synchronization objects
    ReleaseMutex(access);
}

/**
 * returns the number of elements in the {JitterBuffer}.
 *
 * @function   JitterBuffer::size
 *
 * @date       2015-03-28
 *
 * @revision   none
 *
 * @designer   Eric Tsang
 *
 * @programmer Eric Tsang
 *
 * @note       none
 *
 * @signature  int JitterBuffer::size()
 *
 * @return     number of elements in the {JitterBuffer}.
 */
int JitterBuffer::size()
{
    return Heap::size();
}

int JitterBuffer::getElementSize()
{
    return Heap::getElementSize();
}