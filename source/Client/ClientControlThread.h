#ifndef CLIENTCONTROLTHREAD_H_
#define CLIENTCONTROLTHREAD_H_

#include "../Buffer/MessageQueue.h"
#include "ClientWindow.h"
#include <map>

class TCPSocket;

#define IP_ADDR_LEN 16

class ClientControlThread
{
public:
    static ClientControlThread* getInstance();
    void requestDownload(int id);
    void cancelDownload(int id);
    void requestChangeStream(int id);
    void connect(char* ipAddress, unsigned short port);
    void disconnect();
    void setClientWindow( ClientWindow * );
protected:
    ClientControlThread();
    ~ClientControlThread();
    void onDownloadPacket(RequestPacket packet);
    void onChangeStream(RequestPacket packet);
    void onNewSong(SongName song);
private:
	static bool onClose(GuiComponent *_pThis, UINT command, UINT id, WPARAM wParam, LPARAM lParam, INT_PTR *retval);


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
     * reference to the one and only {ClientControlThread} instance.
     */
    ClientWindow * _window;
    /**
     * pointer to the TCPSocket owned by the control thread.
     */
    TCPSocket* tcpSock;
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
    /**
     * list of SongInformation structures sent to client from server
     */
    std::map<int,SongName> _songs;
};

#endif
