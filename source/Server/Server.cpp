#include "Server.h"

Server::Server( unsigned short _tcpPort, unsigned long groupIP, unsigned short udpPort )
    : tcpPort( _tcpPort )
{
    TCPConnections = (TCPConnection *) malloc( ( numTCPConnections + 1 ) * sizeof( TCPConnection ) );
    memset( TCPConnections, 0, sizeof( TCPConnection ) );
    
    memset( &group, 0, sizeof( group ) );
    group.sin_family      = AF_INET;
    group.sin_addr.s_addr = groupIP;
    group.sin_port        = udpPort;
}

Server::~Server()
{
    free( TCPConnections );
}

void Server::startTCP()
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
        MessageBox(NULL, errorStr, L"Error", MB_ICONERROR);
        return;
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
        MessageBox(NULL, errorStr, L"Error", MB_ICONERROR);
        return;
    }
    
    // Put socket in listening state
    if( listen( listenSocket, 5 ) )
    {
        wchar_t errorStr[256] = {0};
        swprintf( errorStr, 256, L"listen() failed: %d", WSAGetLastError() );
        MessageBox(NULL, errorStr, L"Error", MB_ICONERROR);
        return;
    }
    
    // Create worker thread
    CreateThread( NULL          // _In_opt_  LPSECURITY_ATTRIBUTES  lpThreadAttributes
                , 0             // _In_      SIZE_T                 dwStackSize
                , WorkerThread  // _In_      LPTHREAD_START_ROUTINE lpStartAddress
                , (LPVOID) this // _In_opt_  LPVOID                 lpParameter
                , 0             // _In_      DWORD                  dwCreationFlags
                , NULL );       // _Out_opt_ LPDWORD                lpThreadId
    
    // Create accept thread
    CreateThread( NULL          // _In_opt_  LPSECURITY_ATTRIBUTES  lpThreadAttributes
                , 0             // _In_      SIZE_T                 dwStackSize
                , AcceptThread  // _In_      LPTHREAD_START_ROUTINE lpStartAddress
                , (LPVOID) this // _In_opt_  LPVOID                 lpParameter
                , 0             // _In_      DWORD                  dwCreationFlags
                , NULL );       // _Out_opt_ LPDWORD                lpThreadId
    
    
}

DWORD WINAPI AcceptThread( LPVOID lpParam )
{
    Server * server = (Server *) lpParam;
    
    // Start accepting new connections
    while( true )
    {
        if( ( server->TCPConnections[ server->numTCPConnections ].sock
              = accept( server->listenSocket, NULL, NULL ) )
            == INVALID_SOCKET )
        {
            MessageBox(NULL, L"accept() returned with INVALID_SOCKET!", L"Error", MB_ICONERROR);
            break;
        }
        
        if( WSASetEvent( server->newConnectionEvent ) == FALSE )
        {
            wchar_t errorStr[256] = {0};
            swprintf( errorStr, 256, L"WSASetEvent() failed: %d", WSAGetLastError() );
            MessageBox(NULL, errorStr, L"Error", MB_ICONERROR);
            return -1;
        }
    }
}

DWORD WINAPI WorkerThread( LPVOID lpParam )
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
            MessageBox(NULL, errorStr, L"Error", MB_ICONERROR);
            return FALSE;
        }

        // On new connection
        if( retval != WSA_WAIT_IO_COMPLETION )
        {
            if( WSAResetEvent( eventArray[ retval - WSA_WAIT_EVENT_0 ] ) == FALSE )
            {
                wchar_t errorStr[256] = {0};
                swprintf( errorStr, 256, L"WSAResetEvent() failed: %d", WSAGetLastError() );
                MessageBox(NULL, errorStr, L"Error", MB_ICONERROR);
            }
            // TCPConnections[ numTCPConnections ].sock;
            ++server->numTCPConnections;
            
            server->TCPConnections = (TCPConnection *) realloc( server->TCPConnections , ( server->numTCPConnections + 1 ) * sizeof( TCPConnection ) );
            memset( server->TCPConnections + server->numTCPConnections, 0, sizeof( TCPConnection ) );
            // deal with new connection
        }
    }
}

void Server::send( TCPConnection * to
                 , LPWSABUF        lpBuffers
                 , DWORD           dwBufferCount
                 , LPWSAOVERLAPPED lpOverlapped
                 , LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine )
{
    DWORD numberOfBytesSent;
    WSASend( to->sock               // _In_  SOCKET                             s
           , lpBuffers              // _In_  LPWSABUF                           lpBuffers
           , dwBufferCount          // _In_  DWORD                              dwBufferCount
           , &numberOfBytesSent     // _Out_ LPDWORD                            lpNumberOfBytesSent
           , 0                      // _In_  DWORD                              dwFlags
           , lpOverlapped           // _In_  LPWSAOVERLAPPED                    lpOverlapped
           , lpCompletionRoutine ); // _In_  LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
}

void Server::startUDP()
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
        MessageBox(NULL, errorStr, L"Error", MB_ICONERROR);
    }
}

void Server::sendToGroup( const char * buf, int len )
{
    if( sendto( multicastSocket            //_In_ SOCKET                  s
              , buf                        //_In_ const char            * buf
              , len                        //_In_ int                     len
              , 0                          //_In_ int                     flags
              , (struct sockaddr *) &group //_In_ const struct sockaddr * to
              , sizeof( group ) )          //_In_ int                     tolen
        < 0 )
    {
        wchar_t errorStr[256] = {0};
        swprintf( errorStr, 256, L"sendto() failed: %d", WSAGetLastError() );
        MessageBox(NULL, errorStr, L"Error", MB_ICONERROR);
    }
}