#include <windows.h>
#include <vector>

#define MAX_JB_SIZE 5000

class JitterBuffer
{
public:
    JitterBuffer(int elementSize, int delay, int interval);
    int insert(int index, void* src);
    void remove(void* dest);
    /**
     * handle to event that is set when the jitter buffer allows something to be
     *   removed, unset otherwise.
     */
    HANDLE canGet;
private:
    void heapify();
    void trickleDown();
    void swap(int id1, int id2);
    int leftId(int id);
    int rightId(int id);
    int parentId(int id);
    /**
     * holds the last index removed from the jitter buffer
     */
    int lastIndex;
    /**
     * size allocated for the payload of each element in the buffer.
     */
    int elementSize;
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
    /**
     * holds all the data that is in this {JitterBuffer}. the {first} in the
     *   pair in the vector holds the priority of the element, and the {second}
     *   points to the data stored in the element.
     */
    std::vector<std::pair<int,void*>> data;
};
