#include <windows.h>
#include <vector>

class MessageQueue
{
public:
    MessageQueue(int capacity, int elementSize);
    void enqueue(int type, void* src);
    void dequeue(int* type, void* dest);
    /**
     * handle to an event that is set when the queue is not empty; it is unset
     *   otherwise.
     */
    HANDLE hasMessage;
private:
    /**
     * size of an element in the queue.
     */
    int elementSize;
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
