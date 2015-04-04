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
        MessageBox(NULL, errorStr, L"Error", MB_ICONERROR);
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
        MessageBox(NULL, errorStr, L"Error", MB_ICONERROR);
        return false;
    }
    
    // Put socket in listening state
    if( listen( listenSocket, 5 ) )
    {
        wchar_t errorStr[256] = {0};
        swprintf( errorStr, 256, L"listen() failed: %d", WSAGetLastError() );
        MessageBox(NULL, errorStr, L"Error", MB_ICONERROR);
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
            MessageBox(NULL, errorStr, L"Error", MB_ICONERROR);
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
        MessageBox(NULL, errorStr, L"Error", MB_ICONERROR);
		return false;
    }

	return true;
	
	//makes the socket multicast and adds it to the group.
	setsockopt( multicastSocket, IPPROTO_IP, IP_MULTICAST_IF, (char*)&group, sizeof(group));
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

void Server::sendWave(SongName songloc, int speed)
{
	FILE* fp = fopen(songloc.filepath, "rb");
	struct SongStream songInfo;
	char* sendSong;
	int deflen = SIZE_INDEX;
	char* song;

	if (fp) {

		char id[5];
		unsigned long size;
		short format_tag, channels, block_align, bits_per_sample;
		unsigned long format_length, sample_rate, avg_bytes_sec, data_size;
		int data_read = 0;

		fread(id, sizeof(char), 4, fp);
		id[4] = '\0';

		if (!strcmp(id, "RIFF")) {
			fread(&size, sizeof(unsigned long), 1, fp);
			fread(id, sizeof(char), 4, fp);
			id[4] = '\0';

			if (!strcmp(id, "WAVE")) {
				//get wave headers
				fread(id, sizeof(char), 4, fp);
				fread(&format_length, sizeof(unsigned long), 1, fp);
				fread(&format_tag, sizeof(short), 1, fp);
				fread(&channels, sizeof(short), 1, fp);
				fread(&sample_rate, sizeof(unsigned long), 1, fp);
				fread(&avg_bytes_sec, sizeof(unsigned long), 1, fp);
				fread(&block_align, sizeof(short), 1, fp);
				fread(&bits_per_sample, sizeof(short), 1, fp);
				fread(id, sizeof(char), 4, fp);
				fread(&data_size, sizeof(unsigned long), 1, fp);
								
				sendSong = (char*) malloc(sizeof(char) * STREAM_PACKET);

				sendSong[0] = CHANGE_STREAM;
				sendSong[1] = (deflen >> 24) & 0xFF;
				sendSong[2] = (deflen >> 16) & 0xFF;
				sendSong[3] = (deflen >> 8) & 0xFF;
				sendSong[4] = deflen & 0xFF;
				sendSong[5] = (songloc.index >> 24) & 0xFF;
				sendSong[6] = (songloc.index >> 16) & 0xFF;
				sendSong[7] = (songloc.index >> 8) & 0xFF;
				sendSong[8] = songloc.index & 0xFF;
								
				song->data = (char*)malloc(speed + 5);

				//read chunks of data from the file based on the speed selected and send it
				while (data_read = fread(song->data + 5, 1, speed, fp) > 0)
				{
					if (stopSending)
					{
						return;
					}

					//type of message
					song->data[0] = MUSICSTREAM;

					//message len
					song->data[1] = (data_read >> 24) & 0xFF;
					song->data[2] = (data_read >> 16) & 0xFF;
					song->data[3] = (data_read >> 8) & 0xFF;
					song->data[4] = data_read & 0xFF;					
					
					sendToGroup(song->data, data_read + 5);
				}

				stopSending = false;
			}
			else
			{
				cout << "Error: RIFF file but not a wave file\n";
			}
		}
		else 
		{
			cout << "Error: not a RIFF file\n";
		}
	}

	fclose(fp);
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

void Server::stopSong()
{
	stopSending = true;
}