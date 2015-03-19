#include <windows.h>
#include <vector>

class JitterBuffer
{
public:
    JitterBuffer(int _elementSize);
    int insert(long index, void* src);
    int remove(void* dest);
private:
    void heapify();
    void trickleDown();
    void swap(int id1, int id2);
    int left(int id);
    int right(int id);
    int parent(int id);
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
