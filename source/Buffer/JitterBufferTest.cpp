#ifdef DEBUG_JITTERBUFFER

#include "JitterBuffer.h"

int main(void)
{
    int payload;

    JitterBuffer jb(1000,sizeof(int),5000,1000);

    payload = 10;
    jb.put(10,&payload);

    payload = 15;
    jb.put(15,&payload);

    payload = 7;
    jb.put(7,&payload);

    payload = 6;
    jb.put(6,&payload);

    payload = 4;
    jb.put(4,&payload);

    payload = 1;
    jb.put(1,&payload);

    payload = 56;
    jb.put(56,&payload);

    jb.get(&payload);
    printf("payload: %d\n",payload);

    jb.get(&payload);
    printf("payload: %d\n",payload);

    jb.get(&payload);
    printf("payload: %d\n",payload);

    jb.get(&payload);
    printf("payload: %d\n",payload);

    jb.get(&payload);
    printf("payload: %d\n",payload);

    jb.get(&payload);
    printf("payload: %d\n",payload);

    jb.get(&payload);
    printf("payload: %d\n",payload);

    getchar();
}

#endif
