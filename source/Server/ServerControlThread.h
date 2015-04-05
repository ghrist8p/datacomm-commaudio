#ifndef SERVERCONTROLTHREAD_H_
#define SERVERCONTROLTHREAD_H_

#include "../Buffer/MessageQueue.h"



class UDPSocket;

#define IP_ADDR_LEN 16

class ServerControlThread
{
public:
    static ServerControlThread * getInstance();
    void addConnection(TCPSocket* connection);
    void start( unsigned short port );
    void stop();
protected:
    ServerControlThread();
    ~ServerControlThread();
private:
    static DWORD WINAPI _threadRoutine(void* params);
    //static void _handleMsgqMsg(ServerControlThread* dis);
    //static void _handleSockMsgqMsg(ServerControlThread* dis);
    /**
     * pointer to the UDPSocket owned by the control thread.
     */
    UDPSocket * udpSocket;
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
    std::vector<MessageQueue*> _sockMsgqs;
    /**
     * handle to the thread used to run {ServerControlThread::_threadRoutine}.
     */
    HANDLE _thread;
    /**
     * handle to an event object, used to stop the execution of thread.
     */
    HANDLE _threadStopEv;
};

#endif
