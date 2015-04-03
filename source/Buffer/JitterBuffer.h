#ifndef JITTER_BUFFER_H
#define JITTER_BUFFER_H

#include "../common.h"
#include <vector>

#include "Heap.h"

#define MAX_JB_SIZE 5000

class JitterBuffer : private Heap
{
public:
    JitterBuffer(int capacity, int himark, int elementSize, int delay, int interval);
    virtual int put(int index, void* src);
    virtual int get(void* dest);
    virtual int size();
    /**
     * handle to event that is set when the jitter buffer allows something to be
     *   removed, unset otherwise.
     */
    HANDLE canGet;
private:
    /**
     * holds the last index removed from the jitter buffer.
     */
    int lastIndex;
    /**
     * minimum number of elements in the buffer before the buffer stops
     *   regulating the rate that elements can be removed from the buffer.
     */
    int himark;
    /**
     * milliseconds to wait before enabling dequeueing after an element was
     *   inserted, changing the state of the buffer from empty to not empty.
     */
    int delay;
    /**
     * milliseconds to wait before enabling dequeueing after another element was
     *   dequeued.
     */
    int interval;
    /**
     * handle to a semaphore that is 0 then the buffer is empty, positive
     *   otherwise.
     */
    HANDLE notEmpty;
    /**
     * handle to a semaphore that is 100 then the buffer is empty, positive
     *   otherwise.
     */
    HANDLE notFull;
    /**
     * mutex that protects this {JitterBuffer}, and that only one path of
     *   execution is performing operations on the buffer at a time.
     */
    HANDLE access;
};

#endif
