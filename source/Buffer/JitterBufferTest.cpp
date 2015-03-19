
#ifdef DEBUG_JITTERBUFFER

#include "JitterBuffer.h"

int main(void)
{
    int payload;

    JitterBuffer jb(1000,sizeof(int),10,10);

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

#endif
