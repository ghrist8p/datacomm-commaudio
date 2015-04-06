#ifndef HEAP_H
#define HEAP_H

#include "../common.h"
#include <vector>

class Heap
{
public:
    Heap(int capacity, int elementSize);
    virtual void insert(int index, void* src);
    virtual void remove(int* index, void* dest);
    virtual void remove();
    virtual void peek(int* index, void* dest);
    virtual int size();
    virtual int getElementSize();
    virtual void setRelativeZero(int zero);
private:
    void heapify();
    void trickleDown();
    void swap(int id1, int id2);
    int leftId(int id);
    int rightId(int id);
    int parentId(int id);
    int comapreIndexes(int index1, int index2);
    /**
     * the zero is what the heap treats as "zero", it affects how the heap is
     *   sorted internally. for example, if zero is actually zero, then -1 will
     *   be treated as the biggest number that can exist in the heap.
     */
    int relativeZero;
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

#endif
