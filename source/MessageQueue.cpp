#include "MessageQueue.h"

struct Node
{
    int type;
    void* data;
};

MessageQueue::MessageQueue(int _elementSize) : elementSize(_elementSize)
{
    hasMessage = CreateEvent(NULL,TRUE,FALSE,NULL);
    canEnqueue = CreateSemaphore(NULL,100,100,NULL);
    canDequeue = CreateSemaphore(NULL,0,100,NULL);
    access = CreateMutex(NULL, FALSE, NULL);
}

void MessageQueue::enqueue(int type, void* src)
{
    // allocate a node
    Node* n = (Node*) malloc(sizeof(Node));
    void* data = malloc(elementSize);

    // put the data into a node
    memcpy(data,src,elementSize);
    n->type = type;
    n->data = data;

    // obtain synchronization objects
    WaitForSingleObject(canEnqueue,INFINITE);
    WaitForSingleObject(access,INFINITE);

    // put the node into the queue
    messages.push_back(n);

    // now that the element is enqueued, it can be dequeued;
    // set the hasMessage event
    SetEvent(hasMessage);

    // release synchronization objects
    ReleaseMutex(access);
    ReleaseSemaphore(canDequeue,1,NULL);
}

void MessageQueue::dequeue(int* type, void* dest)
{
    // obtain synchronization objects
    WaitForSingleObject(canDequeue,INFINITE);
    WaitForSingleObject(access,INFINITE);

    // remove the element from the queue
    auto it = messages.begin();
    Node* n = *it;
    messages.erase(it);

    // if the queue is empty, reset hasMessage
    if(messages.size() == 0)
    {
        ResetEvent(hasMessage);
    }

    // release synchronization objects
    ReleaseMutex(access);
    ReleaseSemaphore(canEnqueue,1,NULL);

    // get the data from the element
    memcpy(dest,n->data,elementSize);
    *type = n->type;

    // deallocate the element
    free(n);
}
