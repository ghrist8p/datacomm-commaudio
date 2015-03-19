#include "JitterBuffer.h"

#include <map>
#include <cmath>
#include <stdio.h>

#define KEY_CURR_LEVEL 0
#define KEY_PREV_LEVEL_FIRST 1
#define KEY_CURR_LEVEL_FIRST 2
#define KEY_NEXT_LEVEL_FIRST 3

static void fatal_error(char* errmsg);
static std::map<char,int> findLevel(int id);

JitterBuffer::JitterBuffer(int _elementSize)
{
    this->elementSize = _elementSize;
}

void JitterBuffer::insert(long index, void* src)
{
    // put the new element into the heap
    void* payload = malloc(elementSize);
    memcpy(payload,src,elementSize);
    data.emplace_back(index,payload);

    // maintain the heap structure
    heapify();
}

void JitterBuffer::remove(void* dest)
{
    // copy data from root to destination
    memcpy(dest,data[0].second,elementSize);

    // swap the root element with the last element
    swap(0,data.size()-1);

    // remove the last element (originally root) from the heap
    data.erase(data.end());

    // maintain the heap structure
    trickleDown();
}

/**
 * reorganizes the heap so that the last inserted element is moved to the right
 *   place in the heap.
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
}

/**
 * reorganizes the heap so that the out of place element at the root of the heap
 *   is moved to the proper place in the heap.
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
    // int currId = 0;

    // while(?)
    // {
    //     int left  = left(currId);
    //     int right = right(currId);
    //     if(data[left].first > data[right].first)
    //     {
    //         swap(left,currId);
    //         currId = left;
    //     }
    //     else
    //     {
    //         swap(right,currId);
    //         currId = right;
    //     }
    // }
}

void JitterBuffer::swap(int id1, int id2)
{
    auto temp = data[id1];
    data[id1] = data[id2];
    data[id2] = temp;
}

int JitterBuffer::left(int id)
{
    // calculate and return id of left child
    auto result = findLevel(id);
    int nextLevelOffset = (id-result[KEY_CURR_LEVEL_FIRST])*2;
    int left = result[KEY_NEXT_LEVEL_FIRST]+nextLevelOffset;
    return (left >= 0 && left < data.size()) ? left : -1;
}

int JitterBuffer::right(int id)
{
    // calculate and return id of left child
    auto result = findLevel(id);
    int nextLevelOffset = (id-result[KEY_CURR_LEVEL_FIRST])*2+1;
    int right = result[KEY_NEXT_LEVEL_FIRST]+nextLevelOffset;
    return (right >= 0 && right < data.size()) ? right : -1;
}

int JitterBuffer::parent(int id)
{
    // calculate and return id of parent element
    auto result = findLevel(id);
    int currLevelOffset = id-result[KEY_CURR_LEVEL_FIRST];
    int prevLevelOffset = (int) ceil(currLevelOffset/2);
    int parent = result[KEY_PREV_LEVEL_FIRST]+prevLevelOffset;
    return (parent >= 0 && parent < data.size()) ? parent : -1;
}

static std::map<char,int> findLevel(int id)
{
    std::map<char,int> ret;
    ret[KEY_CURR_LEVEL]       = 0;
    ret[KEY_PREV_LEVEL_FIRST] = -1;
    ret[KEY_CURR_LEVEL_FIRST] = 0;
    ret[KEY_NEXT_LEVEL_FIRST] = 1;

    while(ret[KEY_NEXT_LEVEL_FIRST]-1 <= id)
    {
        ++ret[KEY_CURR_LEVEL];
        ret[KEY_PREV_LEVEL_FIRST] = ret[KEY_CURR_LEVEL_FIRST];
        ret[KEY_CURR_LEVEL_FIRST] = ret[KEY_NEXT_LEVEL_FIRST]-1;
        ret[KEY_NEXT_LEVEL_FIRST] *= 2;
    }
    --ret[KEY_NEXT_LEVEL_FIRST];

    return ret;
}

static void fatal_error(char* errmsg)
{
    OutputDebugString(errmsg);
    exit(0);
}

int main(void)
{
    char output[1024];

    printf("left(3):   %2d; %s\n",left(3),  (left(3) == 7)   ? "pass" : "fail");
    printf("right(3):  %2d; %s\n",right(3), (right(3) == 8)  ? "pass" : "fail");
    printf("parent(3): %2d; %s\n",parent(3),(parent(3) == 1) ? "pass" : "fail");
    printf("left(5):   %2d; %s\n",left(5),  (left(5) == 11)  ? "pass" : "fail");
    printf("right(5):  %2d; %s\n",right(5), (right(5) == 12) ? "pass" : "fail");
    printf("parent(5): %2d; %s\n",parent(5),(parent(5) == 2) ? "pass" : "fail");

    getchar();
}
