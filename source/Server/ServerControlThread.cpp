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
    : _msgq(MSGQ_CAPACITY,MSGQ_ELEM_SIZE)
    , _socks()
    , access( CreateMutex(NULL, FALSE, NULL) )
{
    // initialize instance variables
    _threadStopEv = CreateEvent(NULL,TRUE,FALSE,NULL);
    _thread       = INVALID_HANDLE_VALUE;

    _sockHandles.emplace_back( _threadStopEv );
}

ServerControlThread::~ServerControlThread()
{

}


void ServerControlThread::setPlaylist( Playlist * _playlist )
{
    if( _playlist != NULL )
    {
        WaitForSingleObject(access,INFINITE);
        playlist = _playlist;
        ReleaseMutex(access);
    }
}

void ServerControlThread::setUDPSocket( UDPSocket * sock )
{
    if( sock != NULL )
    {
        WaitForSingleObject(access,INFINITE);
        udpSocket = sock;
        ReleaseMutex(access);
    }
}


void ServerControlThread::addConnection( TCPSocket * connection )
{
    WaitForSingleObject(access,INFINITE);
    _socks.emplace_back( connection );
    _sockHandles.emplace_back( connection->getMessageQueue()->hasMessage );
    ReleaseMutex(access);
}

void ServerControlThread::start()
{
    startRoutine(&_thread,_threadStopEv,_threadRoutine,this);
}

void ServerControlThread::stop()
{
    stopRoutine(&_thread,_threadStopEv);
}

DWORD WINAPI ServerControlThread::_threadRoutine( void * params )
{
    ServerControlThread * thiz = (ServerControlThread *) params;

    int breakLoop = FALSE;
    while(!breakLoop)
    {
        int handleNum = 0;
        switch( handleNum = WaitForMultipleObjects(  thiz->_sockHandles.size()
                                                  , &thiz->_sockHandles[ 0 ]
                                                  , FALSE
                                                  , INFINITE ) )
        {
        case WAIT_OBJECT_0+0:   // stop event triggered
            breakLoop = TRUE;
            break;
        default:
            int len = thiz->_socks[ handleNum ]->getMessageQueue()->peekLen();
            RequestPacket * data = new RequestPacket;
            int type;
            thiz->_socks[ handleNum ]->getMessageQueue()->dequeue( &type, data );
            switch( type )
            {
            case CHANGE_STREAM:
                thiz->_handleMsgChangeStream( data );
                break;
            case REQUEST_DOWNLOAD:
                thiz->_handleMsgRequestDownload( data );
                break;
            case CANCEL_DOWNLOAD:
                thiz->_handleMsgCancelDownload( data );
                break;
            case DISCONNECT:
                thiz->_handleMsgDisconnect( handleNum );
                break;
            }
            break;
        }
    }
    return 0;
}

void ServerControlThread::_handleMsgChangeStream( RequestPacket * data )
{
    udpSocket->stopSong();
    DWORD useless;
    _multicastThread = CreateThread( 0, 0, _multicastRoutine, playlist->getSong( data->index ), 0, &useless );
}

void ServerControlThread::_handleMsgRequestDownload( RequestPacket * data )
{

}

void ServerControlThread::_handleMsgCancelDownload( RequestPacket * data )
{

}

void ServerControlThread::_handleMsgDisconnect( int client )
{
    WaitForSingleObject( access, INFINITE );
    _socks.erase( _socks.begin() + client );
    _sockHandles.erase( _sockHandles.begin() + client );
    ReleaseMutex( access );
}

DWORD WINAPI ServerControlThread::_multicastRoutine( void * params )
{
    ServerControlThread * thiz = ServerControlThread::getInstance();
    thiz->udpSocket->sendWave( *((SongName *) params), 60, thiz->_socks );
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
