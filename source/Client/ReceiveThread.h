#ifndef _RECEIVE_THREAD_H_
#define _RECEIVE_THREAD_H_

#include "../common.h"

class MessageQueue;
class JitterBuffer;

class ReceiveThread
{
public:
    ReceiveThread(
        MessageQueue* sockMsgQueue,
        JitterBuffer* musicJitterBuffer);
    ~ReceiveThread();
    void start();
    void stop();
private:
    static DWORD WINAPI threadRoutine(void* params);
    static void handleMsgqMsg(ReceiveThread* dis);
    MessageQueue* sockMsgQueue;
    JitterBuffer* musicJitterBuffer;
    HANDLE thread;
    HANDLE threadStopEv;
};

#endif
