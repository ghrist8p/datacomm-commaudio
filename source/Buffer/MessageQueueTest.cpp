#include "MessageQueue.h"

#ifdef TEST_MESSAGE_QUEUE

DWORD WINAPI producer(void* params)
{
    MessageQueue* msgq = (MessageQueue*) params;

    int repeat = 10000;
    int payload;

    while(--repeat > 0)
    {
        payload = repeat;
        msgq->enqueue(repeat,&payload);
    }

    return 0;
}

DWORD WINAPI consumer(void* params)
{
    MessageQueue* msgq = (MessageQueue*) params;

    int repeat = 10000;
    int type;
    int payload;

    while(--repeat > 0)
    {
        msgq->dequeue(&type,&payload);
        printf("%d:%d\n",type,payload);
    }

    return 0;
}

#define NUM_PRODUCERS 2
#define NUM_CONSUMERS 2

int main(void)
{
    DWORD unused;
    MessageQueue msgq(4);
    HANDLE threads[NUM_PRODUCERS+NUM_CONSUMERS];

    // create threads
    for(int i = 0; i < NUM_PRODUCERS; ++i)
        threads[i] = CreateThread(NULL,0,consumer,&msgq,0,&unused);
    for(int i = 0; i < NUM_CONSUMERS; ++i)
        threads[i+NUM_PRODUCERS] = CreateThread(NULL,0,producer,&msgq,0,&unused);

    // wait for all threads to finish
    WaitForMultipleObjects(NUM_PRODUCERS+NUM_CONSUMERS,threads,TRUE,INFINITE);

    // wait for 10 seconds...
    Sleep(5000);

    return 0;
}

#endif
