// all TODOs in this file...
// TODO: #include "../Net/TCPSocket.h"
// TODO: be able to include the "../Net/TCPSocket.h", and use the DATA_BUFSIZE
// TODO: this should be defined elsewhere, because its part of a protocol
// TODO: implement stufffff!!!!!
// TODO: implement stufffff!!!!!
// TODO: implement stufffff!!!!!
// TODO: TCPSocket->send blahblahblah
// TODO: TCPSocket->send blahblahblah
// TODO: TCPSocket->send blahblahblah
// TODO: TCPSocket->send blahblahblah
// TODO: parse packet, and fill in callback parameters
// TODO: parse packet, and fill in callback parameters
// TODO: parse packet, and fill in callback parameters

#include "ClientControlThread.h"
#include "Sockets.h"
#include "../handlerHelper.h"
#include "../protocol.h"

/*
 * message queue constructor parameters
 */
#define MSGQ_CAPACITY 30
#define MSGQ_ELEM_SIZE sizeof(MsgqElement)
#define SOCK_MSGQ_CAPACITY 1000
#define SOCK_MSGQ_ELEM_SIZE sizeof(SockMsgqElement)

//////////////////////
// type definitions //
//////////////////////

/**
 * element that is put into the message queue.
 */
union MsgqElement
{
    int index;
    char string[STR_LEN];
};

/**
 * element that is put into the socket message queue
 */
union SockMsgqElement
{
    char data[DATA_BUFSIZE];
};

/////////////////////
// synchronization //
/////////////////////

static HANDLE access = CreateMutex(NULL, FALSE, NULL);

////////////////////////////////////////
// ClientControlThread implementation //
////////////////////////////////////////

ClientControlThread* ClientControlThread::_instance = 0;

/**
 * returns a pointer to the one and only ClientControlThread instance.
 *
 * @return   pointer to the one and only ClientControlThread instance.
 */
ClientControlThread* ClientControlThread::getInstance()
{
    // acquire synchronization objects
    WaitForSingleObject(access,INFINITE);

    // retrieve singleton and make if needed
    if(ClientControlThread::_instance == 0)
    {
        ClientControlThread::_instance = new ClientControlThread();
    }

    // release synchronization objects
    ReleaseMutex(access);

    return ClientControlThread::_instance;
}

/**
 * constructs a new {ClientControlThread} object, and initializes all its
 *   instance variables.
 */
ClientControlThread::ClientControlThread()
    :_msgq(MSGQ_CAPACITY,MSGQ_ELEM_SIZE)
    ,_sockMsgq(SOCK_MSGQ_CAPACITY,SOCK_MSGQ_ELEM_SIZE)
{
    // initialize instance variables
    _threadStopEv = CreateEvent(NULL,TRUE,FALSE,NULL);
    _thread       = INVALID_HANDLE_VALUE;
}

/**
 * destructs the client control thread.
 */
ClientControlThread::~ClientControlThread()
{
    disconnect();
}

/**
 * posts a message to an internal message queue, informing the control thread
 *   that it should send a packet retransmission request.
 *
 * @date     2015-03-28T11:17:45-0800
 *
 * @author   Eric Tsang
 *
 * @param    index   what position the packet is for.
 */
void ClientControlThread::requestPacketRetransmission(int index)
{
    // prepare the element for insertion into the message queue
    MsgqElement element;
    element.index = index;

    // insert the element into the message queue
    _msgq.enqueue((int)PacketType::REQUEST_PACKET,&element);
}

/**
 * posts a message to an internal message queue, informing the control thread
 *   that it should send a request to download a song.
 *
 * @date     2015-03-28T11:19:49-0800
 *
 * @author   Eric Tsang
 *
 * @param    file   [description]
 */
void ClientControlThread::requestDownload(char* file)
{
    // prepare the element for insertion into the message queue
    MsgqElement element;
    memcpy(&element.string,file,STR_LEN);

    // insert the element into the message queue
    _msgq.enqueue((int)PacketType::REQUEST_DOWNLOAD,&element);
}

void ClientControlThread::cancelDownload(char* file)
{
    // prepare the element for insertion into the message queue
    MsgqElement element;
    memcpy(&element.string,file,STR_LEN);

    // insert the element into the message queue
    _msgq.enqueue((int)PacketType::CANCEL_DOWNLOAD,&element);
}

void ClientControlThread::requestChangeStream(char* file)
{
    // prepare the element for insertion into the message queue
    MsgqElement element;
    memcpy(&element.string,file,STR_LEN);

    // insert the element into the message queue
    _msgq.enqueue((int)PacketType::CHANGE_STREAM,&element);
}

void ClientControlThread::connect(char* ipAddress, unsigned short port)
{
    // copy connection parameters into the object
    memcpy(this->ipAddress,ipAddress,IP_ADDR_LEN);
    this->port = port;

    // start the threaded routine
    _startRoutine(&_thread,_threadStopEv,_threadRoutine,this);
}

void ClientControlThread::disconnect()
{
    _stopRoutine(&_thread,_threadStopEv);
}

void ClientControlThread::onDownloadPacket(int index, void* data, int len)
{
    // TODO: implement stufffff!!!!!
}

void ClientControlThread::onRetransmissionPacket(int index, void* data, int len)
{
    // TODO: implement stufffff!!!!!
}

void ClientControlThread::onChangeStream(char* file)
{
    // TODO: implement stufffff!!!!!
}

int ClientControlThread::_startRoutine(HANDLE* thread, HANDLE stopEvent,
    LPTHREAD_START_ROUTINE routine, void* params)
{
    // return immediately if the routine is already running
    if(*thread != INVALID_HANDLE_VALUE)
    {
        return 1;
    }

    // reset the stop event
    ResetEvent(stopEvent);

    // start the thread & return
    DWORD useless;
    *thread = CreateThread(0,0,routine,params,0,&useless);
    return (*thread == INVALID_HANDLE_VALUE);
}

int ClientControlThread::_stopRoutine(HANDLE* thread, HANDLE stopEvent)
{
    // return immediately if the routine is already stopped
    if(*thread == INVALID_HANDLE_VALUE)
    {
        return 1;
    }

    // set the stop event to stop the thread
    SetEvent(stopEvent);
    WaitForSingleObject(*thread,INFINITE);

    // invalidate thread handle, so we know it's terminated
    *thread = INVALID_HANDLE_VALUE;
    return 0;
}

DWORD WINAPI ClientControlThread::_threadRoutine(void* params)
{
    printf("Thread started...\n");

    // parse thread parameters
    ClientControlThread* dis = (ClientControlThread*) params;

    // connect to the remote host
    dis->tcpSock = new TCPSocket(dis->ipAddress,dis->port,&dis->_sockMsgq);

    // perform the thread routine
    int breakLoop = FALSE;
    while(!breakLoop)
    {
        HANDLE handles[] = {
            dis->_threadStopEv,
            dis->_msgq.hasMessage,
            dis->_sockMsgq.hasMessage
        };
        switch(WaitForMultipleObjects(3,handles,FALSE,INFINITE))
        {
        case WAIT_OBJECT_0+0:   // stop event triggered
            breakLoop = TRUE;
            break;
        case WAIT_OBJECT_0+1:   // message queue has message
            _handleMsgqMsg(dis);
            break;
        case WAIT_OBJECT_0+2:   // socket received something
            _handleSockMsgqMsg(dis);
            break;
        default:
            fatalError("ClientControlThread::_threadRoutine WaitForMultipleObjects");
            break;
        }
    }

    // disconnect from remote host
    delete dis->tcpSock;

    // return...
    printf("Thread stopped...\n");
    return 0;
}

void ClientControlThread::_handleMsgqMsg(ClientControlThread* dis)
{
    TCHAR s[256];

    // allocate memory to hold message queue message
    PacketType msgType;
    MsgqElement element;

    // get the message queue message
    dis->_msgq.dequeue((int*)&msgType,&element);

    // process the message queue message according to its type
    switch(msgType)
    {
    case PacketType::REQUEST_PACKET:
    {
        RequestPacket packet;
        packet.index = element.index;
        dis->tcpSock->Send((char)PacketType::REQUEST_PACKET,&packet,sizeof(packet));
        break;
    }
    case PacketType::REQUEST_DOWNLOAD:
    {
        StringPacket packet;
        memcpy(packet.string,element.string,STR_LEN);
        dis->tcpSock->Send((char)PacketType::REQUEST_DOWNLOAD,&packet,sizeof(packet));
        break;
    }
    case PacketType::CANCEL_DOWNLOAD:
    {
        StringPacket packet;
        memcpy(packet.string,element.string,STR_LEN);
        dis->tcpSock->Send((char)PacketType::CANCEL_DOWNLOAD,&packet,sizeof(packet));
        break;
    }
    case PacketType::CHANGE_STREAM:
    {
        StringPacket packet;
        memcpy(packet.string,element.string,STR_LEN);
        dis->tcpSock->Send((char)PacketType::CHANGE_STREAM,&packet,sizeof(packet));
        break;
    }
    default:
        fprintf(stderr,"WARNING: received unknown message type: %d\n",msgType);
        break;
    }
}

void ClientControlThread::_handleSockMsgqMsg(ClientControlThread* dis)
{
    // allocate memory to hold message queue message
    PacketType msgType;
    MsgqElement element;

    // get the message queue message
    dis->_sockMsgq.dequeue((int*)&msgType,&element);

    // process the message queue message according to its type
    switch(msgType)
    {
    case PacketType::DOWNLOAD_PACKET:
        OutputDebugString(L"PacketType::DOWNLOAD_PACKET\n");
        // TODO: parse packet, and fill in callback parameters
        dis->onDownloadPacket(0,0,0);
        break;
    case PacketType::RETRANSMISSION_PACKET:
        OutputDebugString(L"PacketType::RETRANSMISSION_PACKET\n");
        // TODO: parse packet, and fill in callback parameters
        dis->onRetransmissionPacket(0,0,0);
        break;
    case PacketType::CHANGE_STREAM:
        OutputDebugString(L"PacketType::CHANGE_STREAM\n");
        // TODO: parse packet, and fill in callback parameters
        dis->onChangeStream(0);
        break;
    default:
        fprintf(stderr,"WARNING: received unknown message type: %d\n",msgType);
        break;
    }
}

/*
int main(void)
{
    ClientControlThread cct;
    cct.start();
    cct.start();
    cct.start();
    cct.start();
    Sleep(10000);
    cct.stop();
    cct.stop();
    cct.stop();
    cct.stop();
    Sleep(1000);
    return 0;
}
*/
