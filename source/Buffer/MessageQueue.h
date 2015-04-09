#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include "../common.h"
#include <vector>

class MessageQueue
{
public:
    MessageQueue(int capacity, int elementSize);
    void enqueue(int type, void* src);
    void enqueue(int type, void* src, int len);
    int peekLen();
    void dequeue(int* type, void* dest);
    void dequeue(int* type, void* dest, int* len);
    int size();
	void clear();
    /**
     * handle to an event that is set when the queue is not empty; it is unset
     *   otherwise.
     */
    HANDLE hasMessage;
    /**
     * size of an element in the queue.
     */
    const int elementSize;
private:
    /**
     * holds the elements of the queue.
     */
    std::vector<struct Node*> messages;
    /**
     * handle to a semaphore that is 0 then the queue is empty, positive
     *   otherwise.
     */
    HANDLE canDequeue;
    /**
     * handle to a semaphore that is 100 then the queue is empty, positive
     *   otherwise.
     */
    HANDLE canEnqueue;
    /**
     * mutex that protects this queue.
     */
    HANDLE access;
};

#endif
