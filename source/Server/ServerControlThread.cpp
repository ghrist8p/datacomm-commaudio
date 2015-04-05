#include "ServerControlThread.h"

/*
 * message queue constructor parameters
 */
#define MSGQ_CAPACITY 30
#define MSGQ_ELEM_SIZE sizeof(MsgqElement)
#define SOCK_MSGQ_CAPACITY 1000
#define SOCK_MSGQ_ELEM_SIZE sizeof(SockMsgqElement)

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

/////////////////////////////////////////
// static function forward declaration //
/////////////////////////////////////////
static int startRoutine(HANDLE* thread, HANDLE stopEvent,
    LPTHREAD_START_ROUTINE routine, void* params);
static int stopRoutine(HANDLE* thread, HANDLE stopEvent);

ServerControlThread * ServerControlThread::getInstance()
{
    static ServerControlThread * _instance = new ServerControlThread();
    return _instance;
}

ServerControlThread::ServerControlThread()
    :_msgq(MSGQ_CAPACITY,MSGQ_ELEM_SIZE)
    ,_sockMsgqs()
{
    // initialize instance variables
    _threadStopEv = CreateEvent(NULL,TRUE,FALSE,NULL);
    _thread       = INVALID_HANDLE_VALUE;
}

ServerControlThread::~ServerControlThread()
{

}


void ServerControlThread::addConnection( TCPSocket * connection )
{
    _sockMsgqs.emplace_back( connection->getMessageQueue() );
}

void ServerControlThread::start(unsigned short port)
{
    this->port = port;
    startRoutine(&_thread,_threadStopEv,_threadRoutine,this);
}

void ServerControlThread::stop()
{
    stopRoutine(&_thread,_threadStopEv);
}

DWORD WINAPI ServerControlThread::_threadRoutine( void * params )
{
    return 0;
}

/////////////////////////////////////
// static function implementations //
/////////////////////////////////////

int startRoutine( HANDLE                 * thread
                , HANDLE                   stopEvent
                , LPTHREAD_START_ROUTINE   routine
                , void                   * params)
{
    // return immediately if the routine is already running
    if( *thread != INVALID_HANDLE_VALUE )
        return 1;

    // reset the stop event
    ResetEvent( stopEvent );

    // start the thread & return
    DWORD useless;
    *thread = CreateThread( 0, 0, routine, params, 0, &useless );
    return ( *thread == INVALID_HANDLE_VALUE );
}

int stopRoutine(HANDLE* thread, HANDLE stopEvent)
{
    // return immediately if the routine is already stopped
    if(*thread == INVALID_HANDLE_VALUE)
        return 1;

    // set the stop event to stop the thread
    SetEvent( stopEvent );
    WaitForSingleObject( *thread, INFINITE );

    // invalidate thread handle, so we know it's terminated
    *thread = INVALID_HANDLE_VALUE;
    return 0;
}
