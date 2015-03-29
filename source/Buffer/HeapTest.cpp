#ifdef DEBUG_HEAP

#include "Heap.h"

int main(void)
{
    int payload;

    printf("RUNNING HeapTest.cpp\n");

    Heap h(1000,sizeof(int));

    payload = 10;
    h.insert(10,&payload);

    payload = 15;
    h.insert(15,&payload);

    payload = 7;
    h.insert(7,&payload);

    payload = 6;
    h.insert(6,&payload);

    payload = 4;
    h.insert(4,&payload);

    payload = 1;
    h.insert(1,&payload);

    payload = 56;
    h.insert(56,&payload);

    h.remove(0,&payload);
    printf("payload: %d\n",payload);

    h.remove(0,&payload);
    printf("payload: %d\n",payload);

    h.remove(0,&payload);
    printf("payload: %d\n",payload);

    h.remove(0,&payload);
    printf("payload: %d\n",payload);

    h.remove(0,&payload);
    printf("payload: %d\n",payload);

    h.remove(0,&payload);
    printf("payload: %d\n",payload);

    h.remove(0,&payload);
    printf("payload: %d\n",payload);

    getchar();
}

#endif
