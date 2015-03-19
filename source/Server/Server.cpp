#include "Server.h"

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
            char errorStr[256] = {0};
            sprintf( errorStr, "WSAWaitForMultipleEvents() failed: %d", WSAGetLastError() );
            MessageBox(NULL, errorStr, L"Error", MB_ICONERROR);
            return FALSE;
        }

        // On new connection
        if( retval != WSA_WAIT_IO_COMPLETION )
        {
            if( WSAResetEvent( eventArray[ retval - WSA_WAIT_EVENT_0 ] ) == FALSE )
            {
                char errorStr[256] = {0};
                sprintf( errorStr, "WSAResetEvent() failed: %d", WSAGetLastError() );
                MessageBox(NULL, errorStr, L"Error", MB_ICONERROR);
            }
            // deal with new connection
        }
    }
}

Server::Server( unsigned short _tcpPort, unsigned short _udpPort )
    : tcpPort( _tcpPort )
    , udpPort( _udpPort )
{
    
}

void Server::start()
{
    // Declare and create listening socket.
    SOCKET listenSocket;
    
    if( ( listenSocket = WSASocket( AF_INET               // _In_ int                af
                                  , SOCK_STREAM           // _In_ int                type
                                  , 0                     // _In_ int                protocol
                                  , NULL                  // _In_ LPWSAPROTOCOL_INFO lpProtocolInfo
                                  , 0                     // _In_ GROUP              g
                                  , WSA_FLAG_OVERLAPPED ) // _In_ DWORD              dwFlags
        ) == INVALID_SOCKET
      )
    {
        char errorStr[256] = {0};
        sprintf( errorStr, "ERROR: creating listen socket: %d", WSAGetLastError() );
        MessageBox(NULL, errorStr, L"Error", MB_ICONERROR);
        return;
    }
    
    // Declare and initialize address and bind to it
    SOCKADDR_IN InternetAddr;
    
    InternetAddr.sin_family      = AF_INET;
    InternetAddr.sin_addr.s_addr = htonl( INADDR_ANY );
    InternetAddr.sin_port        = htoms( server->port );
    
    if( bind( listenSocket
            , (PSOCKADDR) &InternetAddr
            , sizeof( InternetAddr )
            ) == SOCKET_ERROR
      )
    {
        char errorStr[256] = {0};
        sprintf( errorStr, "ERROR: binding listen socket: %d", WSAGetLastError() );
        MessageBox(NULL, errorStr, L"Error", MB_ICONERROR);
        return;
    }
    
    // Put socket in listening state
    if( listen( listenSocket, 5 ) )
    {
        char errorStr[256] = {0};
        sprintf( errorStr, "listen() failed: %d", WSAGetLastError() );
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
    
    // Start accepting new connections
    while( true )
    {
        if( ( newConnection = accept( listenSocket, NULL, NULL ) ) == INVALID_SOCKET )
        {
            MessageBox(NULL, L"accept() returned with INVALID_SOCKET!", L"Error", MB_ICONERROR);
            break;
        }
        
        if( WSASetEvent( acceptor->acceptEvent ) == FALSE )
        {
            char errorStr[256] = {0};
            sprintf( errorStr, "WSASetEvent() failed: %d", WSAGetLastError() );
            MessageBox(NULL, errorStr, L"Error", MB_ICONERROR);
            return;
        }
    }
}