#ifndef _RECEIVE_THREAD_H_
#define _RECEIVE_THREAD_H_

#include "../common.h"

class UDPSocket;
class JitterBuffer;

class ReceiveThread
{
public:
    ReceiveThread(
        UDPSocket* udpSocket,
        JitterBuffer* musicJitterBuffer);
    ~ReceiveThread();
    void start();
    void stop();
private:
    static DWORD WINAPI threadRoutine(void* params);
    static void handleMsgqMsg(ReceiveThread* dis);
    UDPSocket* udpSocket;
    JitterBuffer* musicJitterBuffer;
    HANDLE thread;
    HANDLE threadStopEv;
};

#endif
