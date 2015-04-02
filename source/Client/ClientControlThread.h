#ifndef CLIENTCONTROLTHREAD_H_
#define CLIENTCONTROLTHREAD_H_

#include "../Buffer/MessageQueue.h"

//class TCPSocket;

#define IP_ADDR_LEN 16

class ClientControlThread
{
public:
    static ClientControlThread* getInstance();
    void requestPacketRetransmission(int index);
    void requestDownload(char* file);
    void cancelDownload(char* file);
    void requestChangeStream(char* file);
    void connect(char* ipAddress, unsigned short port);
    void disconnect();
protected:
    ClientControlThread();
    ~ClientControlThread();
    void onDownloadPacket(int index, void* data, int len);
    void onRetransmissionPacket(int index, void* data, int len);
    void onChangeStream(char* file);
private:
    int _startRoutine(HANDLE* thread, HANDLE stopEvent,
        LPTHREAD_START_ROUTINE routine, void* params);
    int _stopRoutine(HANDLE* thread, HANDLE stopEvent);
    static DWORD WINAPI _threadRoutine(void* params);
    static void _handleMsgqMsg(ClientControlThread* dis);
    static void _handleSockMsgqMsg(ClientControlThread* dis);
    /**
     * reference to the one and only {ClientControlThread} instance.
     */
    static ClientControlThread* _instance;
    /**
     * pointer to the TCPSocket owned by the control thread.
     */
    //TCPSocket* tcpSock;
    /**
     * pointer to IP address of the remote host
     */
    char ipAddress[IP_ADDR_LEN];
    /**
     * pointer to IP address of the remote host
     */
    short port;
    /**
     * Message queue used to communicate to {_thread}, and get it to do tasks.
     */
    MessageQueue _msgq;
    /**
     * Message queue used to receive data from {_TCPSocket}.
     */
    MessageQueue _sockMsgq;
    /**
     * handle to the thread used to run {ClientControlThread::_threadRoutine}.
     */
    HANDLE _thread;
    /**
     * handle to an event object, used to stop the execution of thread.
     */
    HANDLE _threadStopEv;
};

#endif
