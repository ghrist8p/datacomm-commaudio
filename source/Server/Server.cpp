#include "Server.h"

Server::Server( unsigned short _tcpPort, newConnectionHandler _handler, void * _data, unsigned long groupIP, unsigned short udpPort )
    : tcpPort( _tcpPort )
    , handler( _handler )
    , data( _data )
{
	stopSending = false;
    numTCPConnections = 0;
    TCPConnections = (TCPConnection *) malloc( ( numTCPConnections + 1 ) * sizeof( TCPConnection ) );
    memset( TCPConnections, 0, sizeof( TCPConnection ) );
    
    newConnectionEvent = WSACreateEvent();

	channels = 0;
	bitrate = 0;
	sampling = 0;

    memset( &group, 0, sizeof( group ) );
    group.sin_family      = AF_INET;
    group.sin_addr.s_addr = groupIP;
    group.sin_port        = udpPort;
}

Server::~Server()
{
    free( TCPConnections );
}

bool Server::startTCP()
{
    // Create listening socket
    if( ( listenSocket = WSASocket( AF_INET               // _In_ int                af
                                  , SOCK_STREAM           // _In_ int                type
                                  , 0                     // _In_ int                protocol
                                  , NULL                  // _In_ LPWSAPROTOCOL_INFO lpProtocolInfo
                                  , 0                     // _In_ GROUP              g
                                  , WSA_FLAG_OVERLAPPED ) // _In_ DWORD              dwFlags
        ) == INVALID_SOCKET
      )
    {
        wchar_t errorStr[256] = {0};
        swprintf_s( errorStr, 256, L"ERROR: creating listen socket: %d", WSAGetLastError() );
        #ifdef DEBUG
        MessageBox(NULL, errorStr, L"Error", MB_ICONERROR));
        #endif
        return false;
    }
    
    // Declare and initialize address and bind to it
    SOCKADDR_IN InternetAddr;
    
    InternetAddr.sin_family      = AF_INET;
    InternetAddr.sin_addr.s_addr = htonl( INADDR_ANY );
    InternetAddr.sin_port        = htons( tcpPort );
    
    if( bind( listenSocket
            , (PSOCKADDR) &InternetAddr
            , sizeof( InternetAddr )
            ) == SOCKET_ERROR
      )
    {
        wchar_t errorStr[256] = {0};
        swprintf( errorStr, 256, L"ERROR: binding listen socket: %d", WSAGetLastError() );
        #ifdef DEBUG
        MessageBox(NULL, errorStr, L"Error", MB_ICONERROR));
        #endif
        return false;
    }
    
    // Put socket in listening state
    if( listen( listenSocket, 5 ) )
    {
        wchar_t errorStr[256] = {0};
        swprintf( errorStr, 256, L"listen() failed: %d", WSAGetLastError() );
        #ifdef DEBUG
        MessageBox(NULL, errorStr, L"Error", MB_ICONERROR));
        #endif
        return false;
    }
    
    // Create worker thread
    hWorkerThread = CreateThread( NULL          // _In_opt_  LPSECURITY_ATTRIBUTES  lpThreadAttributes
                                , 0             // _In_      SIZE_T                 dwStackSize
                                , WorkerThread  // _In_      LPTHREAD_START_ROUTINE lpStartAddress
                                , (LPVOID) this // _In_opt_  LPVOID                 lpParameter
                                , 0             // _In_      DWORD                  dwCreationFlags
                                , NULL );       // _Out_opt_ LPDWORD                lpThreadId
    
    // Create accept thread
    hAcceptThread = CreateThread( NULL          // _In_opt_  LPSECURITY_ATTRIBUTES  lpThreadAttributes
                                , 0             // _In_      SIZE_T                 dwStackSize
                                , AcceptThread  // _In_      LPTHREAD_START_ROUTINE lpStartAddress
                                , (LPVOID) this // _In_opt_  LPVOID                 lpParameter
                                , 0             // _In_      DWORD                  dwCreationFlags
                                , NULL );       // _Out_opt_ LPDWORD                lpThreadId
    
	return true;
}

DWORD WINAPI Server::AcceptThread( LPVOID lpParam )
{
    Server * server = (Server *) lpParam;
    
    // Start accepting new connections
    while( true )
    {
        if( ( server->TCPConnections[ server->numTCPConnections ].sock
              = accept( server->listenSocket, NULL, NULL ) )
            == INVALID_SOCKET )
        {
            break;
        }
        
        if( WSASetEvent( server->newConnectionEvent ) == FALSE )
        {
            wchar_t errorStr[256] = {0};
            swprintf( errorStr, 256, L"WSASetEvent() failed: %d", WSAGetLastError() );
            #ifdef DEBUG
            MessageBox(NULL, errorStr, L"Error", MB_ICONERROR));
            #endif
            return -1;
        }
    }
}

DWORD WINAPI Server::WorkerThread( LPVOID lpParam )
{
    Server * server = (Server *) lpParam;
    
    WSAEVENT   eventArray[1];
    DWORD retval;

    eventArray[0] = server->newConnectionEvent;

    while( true )
    {
        // Wait for new connection
        // Also put thread in alterable state so it can perform completion routines
        retval = WSAWaitForMultipleEvents( 1            // _In_ DWORD            cEvents
                                         , eventArray   // _In_ const WSAEVENT * lphEvents
                                         , FALSE        // _In_ BOOL             fWaitAll
                                         , WSA_INFINITE // _In_ DWORD            dwTimeout
                                         , TRUE );      // _In_ BOOL             fAlertable
        
        // On error
        if( retval == WSA_WAIT_FAILED )
        {
            wchar_t errorStr[256] = {0};
            swprintf( errorStr, 256, L"WSAWaitForMultipleEvents() failed: %d", WSAGetLastError() );
            #ifdef DEBUG
            MessageBox(NULL, errorStr, L"Error", MB_ICONERROR));
            #endif
            return FALSE;
        }

        // On new connection
        if( retval != WSA_WAIT_IO_COMPLETION )
        {
            if( WSAResetEvent( eventArray[ retval - WSA_WAIT_EVENT_0 ] ) == FALSE )
            {
                wchar_t errorStr[256] = {0};
                swprintf( errorStr, 256, L"WSAResetEvent() failed: %d", WSAGetLastError() );
                #ifdef DEBUG
                MessageBox(NULL, errorStr, L"Error", MB_ICONERROR));
                #endif
            }
            // TCPConnections[ numTCPConnections ].sock;
            ++server->numTCPConnections;
            
            server->TCPConnections = (TCPConnection *) realloc( server->TCPConnections , ( server->numTCPConnections + 1 ) * sizeof( TCPConnection ) );
            memset( server->TCPConnections + server->numTCPConnections, 0, sizeof( TCPConnection ) );
            if( server->handler )
                server->handler( server->TCPConnections + server->numTCPConnections - 1, server->data );
        }
    }
}

void Server::submitCompletionRoutine( PAPCFUNC lpCompletionRoutine, void * to )
{
    QueueUserAPC( lpCompletionRoutine // _In_  PAPCFUNC pfnAPC,
                , hWorkerThread       // _In_  HANDLE hThread,
                , (ULONG_PTR)to );    // _In_  ULONG_PTR dwData
}

bool Server::startUDP()
{
    multicastSocket = WSASocket( AF_INET
                               , SOCK_DGRAM
                               , IPPROTO_UDP
                               , NULL
                               , 0
                               , WSA_FLAG_OVERLAPPED );
    if( multicastSocket == INVALID_SOCKET )
    {
        wchar_t errorStr[256] = {0};
        swprintf( errorStr, 256, L"WSASocket() failed: %d", WSAGetLastError() );
        #ifdef DEBUG
        MessageBox(NULL, errorStr, L"Error", MB_ICONERROR));
        #endif
		return false;
    }

	return true;
	
	//makes the socket multicast and adds it to the group.
	setsockopt( multicastSocket, IPPROTO_IP, IP_MULTICAST_IF, (char*)&group, sizeof(group));
}
void Server::disconnect()
{
	closesocket(listenSocket);
	closesocket(multicastSocket);

	for (int i = 0; i < numTCPConnections; i++)
	{
		closesocket(TCPConnections[i].sock);
	}
	numTCPConnections = 0;
}
