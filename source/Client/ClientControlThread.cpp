#include "ClientControlThread.h"

#define MSGQ_CAPACITY 1000
#define MSGQ_ELEM_SIZE 1000

ClientControlThread::ClientControlThread()
    :_msgq(MSGQ_CAPACITY,MSGQ_ELEM_SIZE)
{

}

ClientControlThread::~ClientControlThread()
{

}

void ClientControlThread::requestPacketRetransmission(int index)
{

}

void ClientControlThread::requestDownload(char* file)
{

}

void ClientControlThread::cancelDownload(char* file)
{

}

void ClientControlThread::changeStream(char* file)
{

}

void ClientControlThread::onDownloadPacket(int index, void* data, int len)
{

}

void ClientControlThread::onRetransmissionPacket(int index, void* data, int len)
{

}

void ClientControlThread::onChangeStream(char* file)
{

}

DWORD WINAPI ClientControlThread::_threadRoutine(void* params)
{

}

int ClientControlThread::startRoutine(HANDLE thread, LPTHREAD_START_ROUTINE routine, void* params)
{
    // return immediately if the routine is already running
    if(*thread != 0)
    {
        return INVALID_OPERATION;
    }

    // create the control pipe
    if(pipe(controlPipe) == SYS_ERROR)
    {
        fatal_error("failed to create the control pipe");
    }

    // start the thread
    pthread_create(thread,0,routine,params);
    return SUCCESS;
}

int ClientControlThread::stopRoutine(HANDLE thread, int* controlPipe)
{
    // return immediately if the routine is already stopped
    if(*thread == 0)
    {
        return INVALID_OPERATION;
    }

    // close the control pipe which terminates the thread
    close(controlPipe[1]);
    pthread_join(*thread,0);

    // set thread to 0, so we know it's terminated
    *thread = 0;
    return SUCCESS;
}
