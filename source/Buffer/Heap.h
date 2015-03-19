#include <windows.h>
#include <vector>

class Heap
{
public:
    Heap(int capacity, int elementSize);
    virtual void insert(int index, void* src);
    virtual void remove(int* index, void* dest);
private:
    void heapify();
    void trickleDown();
    void swap(int id1, int id2);
    int leftId(int id);
    int rightId(int id);
    int parentId(int id);
    /**
     * size allocated for the payload of each element in the buffer.
     */
    int elementSize;
    /**
     * holds all the data that is in this {Heap}. the {first} in the pair in the
     *   vector holds the priority of the element, and the {second} points to
     *   the data stored in the element.
     */
    std::vector<std::pair<int,void*>> data;
};
