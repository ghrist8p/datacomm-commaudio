#include "Heap.h"

Heap::Heap(int capacity, int elementSize)
{
    this->elementSize = elementSize;

    data.reserve(capacity);
}

/**
 * puts the passed data into the jitter buffer if the index of the last element
 *   removed is smaller than the index of the element being inserted (i.e. this
 *   element is being inserted too late, since elements that should be consumed
 *   after it have already been consumed).
 *
 * @function   Heap::insert
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
 * @signature  int Heap::insert(int index, void* src)
 *
 * @param      index index of the element being inserted.
 * @param      src pointer to data to be copied into the element.
 *
 * @return     0 upon success, 1 upon rejection.
 */
void Heap::insert(int index, void* src)
{
    // put the new element into the heap
    void* payload = malloc(elementSize);
    memcpy(payload,src,elementSize);
    data.emplace_back(index,payload);

    // maintain the heap structure
    heapify();
}

/**
 * copies the root element from the heap to the passed user's pointers, then
 *   removed the root element from the heap, while maintaining the heap data
 *   structure.
 *
 * @function   Heap::remove
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
 * @signature  void Heap::remove(int* index, void* dest)
 *
 * @param      index integer to copy the index of the peeked element of.
 * @param      dest holds the data at the index.
 */
void Heap::remove(int* index, void* dest)
{
    // copy data from root to destination
    Heap::peek(index,dest);

    // remove the root element of the heap
    Heap::remove();
}

/**
 * removes the root element from the heap, without saving it.
 *
 * @function   Heap::remove
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
 * @signature  void Heap::remove()
 */
void Heap::remove()
{
    // swap the root element with the last element
    swap(0,data.size()-1);

    // remove the last element (originally root) from the heap
    data.erase(--data.end());

    // maintain the heap structure
    trickleDown();
}

/**
 * takes the information at the root of the heap, and copies it to the user's
 *   space, without modifying the elements in the heap.
 *
 * @function   Heap::peek
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
 * @signature  void Heap::peek(int* index, void* dest)
 *
 * @param      index integer to copy the index of the peeked element of.
 * @param      dest holds the data at the index.
 */
void Heap::peek(int* index, void* dest)
{
    // copy data from root to destination
    if(index != 0)
    {
        *index = data[0].first;
    }
    if(dest != 0)
    {
        memcpy(dest,data[0].second,elementSize);
    }
}

/**
 * returns the number of elements currently in the heap.
 *
 * @function   Heap::size
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
 * @signature  int Heap::size()
 *
 * @return     number of elements currently in the heap.
 */
int Heap::size()
{
    return data.size();
}

int Heap::getElementSize()
{
    return elementSize;
}

/**
 * reorganizes the minimum heap so that the last inserted element is moved to
 *   the right place in the minimum heap.
 *
 * @function   Heap::heapify
 *
 * @date       2015-03-18
 *
 * @revision   none
 *
 * @designer   Eric Tsang
 *
 * @programmer Eric Tsang
 *
 * @note       none
 *
 * @signature  void Heap::heapify()
 */
void Heap::heapify()
{
    int curr = data.size()-1;

    // heapify
    while(true)
    {
        int parent = parentId(curr);
        if(data[curr].first < data[parent].first && parent != -1)
        {
            swap(parent,curr);
            curr = parent;
        }
        else
        {
            break;
        }
    }
}

/**
 * reorganizes the minimum heap so that the out of place element at the root of
 *   the minimum heap is moved to the proper place in the minimum heap.
 *
 * @function   Heap::trickleDown
 *
 * @date       2015-03-18
 *
 * @revision   none
 *
 * @designer   Eric Tsang
 *
 * @programmer Eric Tsang
 *
 * @note       none
 *
 * @signature  void Heap::trickleDown()
 */
void Heap::trickleDown()
{
    int curr = 0;

    while(true)
    {
        int left  = leftId(curr);
        int right = rightId(curr);

        // break if there are no more child nodes
        if(left == -1 && right == -1)
        {
            break;
        }

        // do the trickling
        if((right == -1 && left != -1) || (left != -1 && data[left].first < data[right].first))
        {
            if(data[left].first < data[curr].first)
            {
                swap(left,curr);
                curr = left;
            }
            else
            {
                break;
            }
        }
        else if((left == -1 && right != -1) || (right != -1 && data[right].first <= data[left].first))
        {
            if(data[right].first < data[curr].first)
            {
                swap(right,curr);
                curr = right;
            }
            else
            {
                break;
            }
        }
    }
}

void Heap::swap(int id1, int id2)
{
    auto temp = data[id1];
    data[id1] = data[id2];
    data[id2] = temp;
}

int Heap::leftId(int id)
{
    // calculate and return id of left child
    int left = 2*id+1;
    return (left >= 0 && left < (int) data.size()) ? left : -1;
}

int Heap::rightId(int id)
{
    // calculate and return id of right child
    int right = 2*id+2;
    return (right >= 0 && right < (int) data.size()) ? right : -1;
}

int Heap::parentId(int id)
{
    // calculate and return id of parent element
    int parent = (id-1)/2;
    return (parent >= 0 && parent < (int) data.size()) ? parent : -1;
}
