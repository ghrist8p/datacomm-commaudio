#include "ServerControlThread.h"
#include "../Client/FileTransferer.h"

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
	fileTransferer = new FileTransferer(NULL);

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

Playlist * ServerControlThread::getPlaylist()
{
    return playlist;
}

void ServerControlThread::setUDPSocket( UDPSocket * sock )
{
    if( sock != NULL )
    {
        WaitForSingleObject(access,INFINITE);
        udpSocket = sock;
        udpSocket->setGroup(MULTICAST_ADDR,0);
        ReleaseMutex(access);
    }
}


void ServerControlThread::addConnection( TCPSocket * connection )
{
    WaitForSingleObject(access,INFINITE);
    _socks.emplace_back( connection );
    _sockHandles.emplace_back( connection->getMessageQueue()->hasMessage );
    QueueUserAPC( _sendPlaylistToOne        // _In_  PAPCFUNC pfnAPC,
                , _thread                   // _In_  HANDLE hThread,
                , (ULONG_PTR) connection ); // _In_  ULONG_PTR dwData
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
		handleNum = WaitForMultipleObjectsEx(  thiz->_sockHandles.size()
                                            , &thiz->_sockHandles[ 0 ]
                                            , FALSE
                                            , INFINITE
                                            , TRUE );
		if( handleNum == WAIT_OBJECT_0 + 0 )
		{
            breakLoop = TRUE;
		}
		else if( handleNum > WAIT_OBJECT_0 + 0 && handleNum < WAIT_OBJECT_0 + thiz->_sockHandles.size() )
        {
			TCPSocket * sock = thiz->_socks[ handleNum - 1 ];

            int type;
			TCPPacket packet;
            sock->getMessageQueue()->dequeue( &type, &packet );
            switch( type )
            {
            case CHANGE_STREAM:
				thiz->_handleMsgChangeStream( &packet.requestPacket );
                break;
            case REQUEST_DOWNLOAD:
                thiz->_handleMsgRequestDownload( &packet.requestPacket, sock);
                break;
            case CANCEL_DOWNLOAD:
                thiz->_handleMsgCancelDownload( &packet.requestPacket );
                break;
            case DISCONNECT:
                thiz->_handleMsgDisconnect( handleNum - 1 );
                break;
            }
		}
		else if( handleNum == WAIT_IO_COMPLETION )
		{

		}
		else
		{
			wchar_t errorStr[256] = {0};
			swprintf( errorStr, 256, L"WaitForMultipleObjectsEx() failed: %d\n"
									 L"thiz->_sockHandles.size(): %d", handleNum, thiz->_sockHandles.size() );
			MessageBox(NULL, errorStr, L"Error", MB_ICONERROR);
			return false;
		}
    }
    return 0;
}

void ServerControlThread::_handleMsgChangeStream( RequestPacket * data )
{
    udpSocket->stopSong();
    WaitForSingleObject(_multicastThread,5000);
    DWORD useless;
    _multicastThread = CreateThread( 0, 0, _multicastRoutine, playlist->getSong( data->index ), 0, &useless );
}

void ServerControlThread::_handleMsgRequestDownload( RequestPacket * data, TCPSocket* socket )
{
	fileTransferer->sendFile((char *)playlist->getSong( data->index )->filepath, socket);
}

void ServerControlThread::_handleMsgCancelDownload( RequestPacket * data )
{

}

void ServerControlThread::_handleMsgDisconnect( int client )
{
    WaitForSingleObject( access, INFINITE );
    _socks.erase( _socks.begin() + client );
    _sockHandles.erase( _sockHandles.begin() + client + 1 );
    ReleaseMutex( access );
}

void ServerControlThread::sendPlaylistToAll( void )
{
    QueueUserAPC( _sendPlaylistToAllRoutine // _In_  PAPCFUNC pfnAPC,
                , _thread                   // _In_  HANDLE hThread,
                , NULL );                   // _In_  ULONG_PTR dwData
}

VOID CALLBACK ServerControlThread::_sendPlaylistToAllRoutine( ULONG_PTR )
{
    ServerControlThread * thiz = ServerControlThread::getInstance();
    for( std::vector< TCPSocket * >::iterator sockit = thiz->_socks.begin()
        ; sockit != thiz->_socks.end()
        ; ++sockit )
    {
        _sendPlaylistToOne( (ULONG_PTR) *sockit );
    }
}

VOID CALLBACK ServerControlThread::_sendPlaylistToOne( ULONG_PTR data )
{
    ServerControlThread * thiz = ServerControlThread::getInstance();
    TCPSocket * sock = (TCPSocket *) data;

    for( std::vector< SongName >::iterator songit = thiz->playlist->playlist.begin()
       ; songit != thiz->playlist->playlist.end()
       ; ++songit )
    {
        sock->Send( NEW_SONG, &(*songit), sizeof( SongName ) );
    }
}
//
//DWORD WINAPI ServerControlThread::_sendFileToOne( void * params )
//{
//    ServerControlThread * thiz = ServerControlThread::getInstance();
//    thiz->udpSocket->sendWave( *((SongName *) params), 60, thiz->_socks );
//    return 0;
//}

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
