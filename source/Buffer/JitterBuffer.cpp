#include "JitterBuffer.h"

#include <map>
#include <cmath>
#include <stdio.h>

#define KEY_CURR_LEVEL 0
#define KEY_PREV_LEVEL_FIRST 1
#define KEY_CURR_LEVEL_FIRST 2
#define KEY_NEXT_LEVEL_FIRST 3

JitterBuffer::JitterBuffer(int elementSize, int delay, int interval)
{
    this->lastIndex   = 0;
    this->elementSize = elementSize;
    this->delay       = delay;
    this->interval    = interval;
    this->canGet      = CreateEvent(NULL,TRUE,FALSE,NULL);
    this->access      = CreateMutex(NULL, FALSE, NULL);
    this->notFull     = CreateSemaphore(NULL,MAX_JB_SIZE,MAX_JB_SIZE,NULL);
    this->notEmpty    = CreateSemaphore(NULL,0,MAX_JB_SIZE,NULL);

    data.reserve(MAX_JB_SIZE);
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
 * @return     0 upon success, 1 upon rejection.
 */
int JitterBuffer::insert(int index, void* src)
{
    int ret;

    // acquire synchronization objects
    WaitForSingleObject(notFull,INFINITE);
    WaitForSingleObject(access,INFINITE);

    if(index > lastIndex)
    {
        ret = 0;

        // put the new element into the heap
        void* payload = malloc(elementSize);
        memcpy(payload,src,elementSize);
        data.emplace_back(index,payload);

        // maintain the heap structure
        heapify();
    }
    else
    {
        ret = 1;
    }

    // release synchronization objects
    ReleaseMutex(access);
    ReleaseSemaphore(notEmpty,1,NULL);

    return ret;
}

void JitterBuffer::remove(void* dest)
{
    // acquire synchronization objects
    WaitForSingleObject(canGet,INFINITE);
    WaitForSingleObject(notEmpty,INFINITE);
    WaitForSingleObject(access,INFINITE);

    // copy data from root to destination
    lastIndex = data[0].first;
    memcpy(dest,data[0].second,elementSize);

    // swap the root element with the last element
    swap(0,data.size()-1);

    // remove the last element (originally root) from the heap
    data.erase(--data.end());

    // maintain the heap structure
    trickleDown();

    // release synchronization objects
    ReleaseMutex(access);
    ReleaseSemaphore(notFull,1,NULL);
}

/**
 * reorganizes the minimum heap so that the last inserted element is moved to
 *   the right place in the minimum heap.
 *
 * @function   JitterBuffer::heapify
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
 * @signature  void JitterBuffer::heapify()
 */
void JitterBuffer::heapify()
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
 * @function   JitterBuffer::trickleDown
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
 * @signature  void JitterBuffer::trickleDown()
 */
void JitterBuffer::trickleDown()
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

void JitterBuffer::swap(int id1, int id2)
{
    auto temp = data[id1];
    data[id1] = data[id2];
    data[id2] = temp;
}

int JitterBuffer::leftId(int id)
{
    // calculate and return id of left child
    int left = 2*id+1;
    return (left >= 0 && left < (int) data.size()) ? left : -1;
}

int JitterBuffer::rightId(int id)
{
    // calculate and return id of left child
    int right = 2*id+2;
    return (right >= 0 && right < (int) data.size()) ? right : -1;
}

int JitterBuffer::parentId(int id)
{
    // calculate and return id of parent element
    int parent = (id-1)/2;
    return (parent >= 0 && parent < (int) data.size()) ? parent : -1;
}

int main(void)
{
    int payload;

    JitterBuffer jb(sizeof(int),10,10);

    payload = 10;
    jb.insert(10,&payload);

    payload = 15;
    jb.insert(15,&payload);

    payload = 7;
    jb.insert(7,&payload);

    payload = 6;
    jb.insert(6,&payload);

    payload = 4;
    jb.insert(4,&payload);

    payload = 1;
    jb.insert(1,&payload);

    payload = 56;
    jb.insert(56,&payload);

    jb.remove(&payload);
    printf("payload: %d\n",payload);

    jb.remove(&payload);
    printf("payload: %d\n",payload);

    jb.remove(&payload);
    printf("payload: %d\n",payload);

    jb.remove(&payload);
    printf("payload: %d\n",payload);

    jb.remove(&payload);
    printf("payload: %d\n",payload);

    jb.remove(&payload);
    printf("payload: %d\n",payload);

    jb.remove(&payload);
    printf("payload: %d\n",payload);

    getchar();
}
