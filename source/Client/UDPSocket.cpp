#include "Sockets.h"

#define RUN_TEST

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: UDPSocket
--
-- DATE: March 17, 2015
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Manuel Gonzales
--
-- PROGRAMMER: Manuel Gonzales
--
-- INTERFACE: UDPSocket::UDPSocket(int port, MessageQueue mqueue)
--
--	port : port number to do the bind
--  mqueue : message queue to use for storing the data.
--
--	RETURNS: nothing.
--
--	NOTES:
--  This is the constructor for the UDP socket, it will create the socket and will start the thread to listen for data
----------------------------------------------------------------------------------------------------------------------*/
UDPSocket::UDPSocket(int port, MessageQueue* mqueue)
{
	int error;
	struct sockaddr_in server;
	WSADATA WSAData;
	WORD wVersionRequested;

	HANDLE ThreadHandle;
	DWORD ThreadId;
	msgqueue = mqueue;

	mutex = CreateMutex(NULL, FALSE, NULL);

	wVersionRequested = MAKEWORD(2, 2);
	error = WSAStartup(wVersionRequested, &WSAData);

	if (error != 0) //No usable DLL
	{
		MessageBox(NULL, L"GlobalAlloc() failed with error", L"ERROR", MB_ICONERROR);
		return;
	}

	// Create the socket
	if ((sd = WSASocket(PF_INET, SOCK_DGRAM, IPPROTO_IP, NULL, 0,
		WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
	{
		printf("Socket cannot be created- Read Help guide for more information");
		return;
	}

	// Initialize and set up the address structure
	memset((char *)&server, 0, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	// Copy the server address

	// Connecting to the server
	if (bind(sd, (struct sockaddr *)&server, sizeof(server)) == -1)
	{
		perror("Can't bind name to socket");
		exit(1);
	}

	if ((ThreadHandle = CreateThread(NULL, 0, UDPThread, (void*)this, 0, &ThreadId)) == NULL)
	{
		printf("CreateThread failed with error %d\n", GetLastError());
		return;
	}
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: ~UDPSocket
--
-- DATE: March 17, 2015
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Manuel Gonzales
--
-- PROGRAMMER: Manuel Gonzales
--
-- INTERFACE: UDPSocket::~UDPSocket()
--
--	RETURNS: nothing.
--
--	NOTES:
--  This is the destructor that will do the cleanup.
----------------------------------------------------------------------------------------------------------------------*/
UDPSocket::~UDPSocket()
{
	closesocket(sd);
	WSACleanup();
}


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: Send
--
-- DATE: March 17, 2015
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Manuel Gonzales
--
-- PROGRAMMER: Manuel Gonzales
--
-- INTERFACE: int UDPSocket::Send(void* data, int length, char* dest_ip, int dest_port)
--
--	data : data to send
--  length : data length
--  dest_ip : ip address of the destination
--  dest_port : port number of the destination
--
--	RETURNS: 1 in success, 0 in error.
--
--	NOTES:
--  This will send the desired data to another UDP client socket.
----------------------------------------------------------------------------------------------------------------------*/
int UDPSocket::Send(char type, void* data, int length, char* dest_ip, int dest_port)
{
	DWORD Flags;
	LPSOCKET_INFORMATION SocketInfo;
	DWORD SendBytes;
	DWORD WaitResult;
	struct sockaddr_in destination;
	int destsize = sizeof(destination);
	char* data_send = (char*)malloc(sizeof(char) * (length + 5));

	data_send[0] = type;

	//message len
	data_send[1] = (length >> 24) & 0xFF;
	data_send[2] = (length >> 16) & 0xFF;
	data_send[3] = (length >> 8) & 0xFF;
	data_send[4] = length & 0xFF;

	WaitResult = WaitForSingleObject(mutex, INFINITE);

	if (WaitResult = WAIT_OBJECT_0)
	{
		if ((SocketInfo = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR,
			sizeof(SOCKET_INFORMATION))) == NULL)
		{
			printf("GlobalAlloc() failed with error %d\n", GetLastError());
			return 0;
		}

		SocketInfo->Socket = sd;
		ZeroMemory(&(SocketInfo->Overlapped), sizeof(WSAOVERLAPPED));
		SocketInfo->DataBuf.len = length + 5;
		SocketInfo->DataBuf.buf = data_send;
		Flags = 0;

		destination.sin_addr.s_addr = inet_addr(dest_ip);
		if (destination.sin_addr.s_addr == INADDR_NONE)
		{
			MessageBox(NULL, L"The target ip address entered must be a legal IPv4 address", L"ERROR", MB_ICONERROR);
			return 0;
		}

		destination.sin_port = htons((u_short)dest_port);

		if (destination.sin_port == 0)
		{
			MessageBox(NULL, L"The targetport must be a legal UDP port number", L"ERROR", MB_ICONERROR);
			return 0;
		}

		if (WSASendTo(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &SendBytes, Flags, (SOCKADDR*)&destination, destsize,
			&(SocketInfo->Overlapped), 0) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				MessageBox(NULL, L"WSASend() failed with error", L"ERROR", MB_ICONERROR);
				return 0;
			}
		}
		else
		{
			return 1;
		}
	}
	else
	{
		MessageBox(NULL, L"Error in the mutex", L"ERROR", MB_ICONERROR);
		return 0;
	}

}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: UDPThread
--
-- DATE: March 17, 2015
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Manuel Gonzales
--
-- PROGRAMMER: Manuel Gonzales
--
-- INTERFACE: DWORD WINAPI UDPSocket::UDPThread(LPVOID lpParameter)
--
--	lpParameter : instance of the object
--
--	RETURNS: nothing.
--
--	NOTES:
--  This will call the start Thread fucntion
----------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI UDPSocket::UDPThread(LPVOID lpParameter)
{
	UDPSocket* This = (UDPSocket*)lpParameter;
	return This->ThreadStart();
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: ThreadStart
--
-- DATE: March 17, 2015
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Manuel Gonzales
--
-- PROGRAMMER: Manuel Gonzales
--
-- INTERFACE: DWORD UDPSocket::ThreadStart(void)
--
--	RETURNS: FALSE in error.
--
--	NOTES:
--  This function will start receiving data from the socket nad placing it into the message queue based on types.
----------------------------------------------------------------------------------------------------------------------*/
DWORD UDPSocket::ThreadStart(void)
{
	DWORD Flags;
	LPSOCKET_INFORMATION SocketInfo;
	DWORD RecvBytes;
	int flag = 0;
	int msg_type = 0;
	int len = 0;
	struct sockaddr_in source;
	int length = sizeof(struct sockaddr_in);


	if ((SocketInfo = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR,
		sizeof(SOCKET_INFORMATION))) == NULL)
	{
		MessageBox(NULL, L"GlobalAlloc() failed with error", L"ERROR", MB_ICONERROR);
		return FALSE;
	}

	SocketInfo->Socket = sd;
	ZeroMemory(&(SocketInfo->Overlapped), sizeof(WSAOVERLAPPED));
	SocketInfo->DataBuf.buf = SocketInfo->Buffer;
	SocketInfo->mqueue = msgqueue;
	Flags = 0;

	while (true)
	{
    	SocketInfo->DataBuf.len = DATA_BUFSIZE;

		if (WSARecvFrom(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &RecvBytes, &Flags, (sockaddr*)&source,
			&length, 0, 0) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				MessageBox(NULL, L"WSARecv() failed with error", L"ERROR", MB_ICONERROR);
				return FALSE;
			}
		}
		else
		{
			len = (SocketInfo->Buffer[1] << 24) | (SocketInfo->Buffer[2] << 16) | (SocketInfo->Buffer[3] << 8) | (SocketInfo->Buffer[4]);
			CHAR* dataReceived = (char*)malloc(sizeof(char) * len);
			memoryCopy(dataReceived, SocketInfo->Buffer+5, len);
			char* sourceaddr = inet_ntoa(source.sin_addr);

			switch (SocketInfo->Buffer[0])
			{
				case MUSICSTREAM:
					SocketInfo->mqueue->enqueue(MUSICSTREAM, dataReceived);
					break;

				case MICSTREAM:
					SocketInfo->mqueue->enqueue(MICSTREAM, dataReceived);
					break;

				default:
					MessageBox(NULL, L"Unknown Type of Message Received", L"ERROR", MB_ICONERROR);
			}

			free(dataReceived);
		}
	}
}

void UDPSocket::setGroup(char* group_address)
{
    memset(&mreq,0,sizeof(mreq));
	mreq.imr_multiaddr.s_addr = inet_addr(group_address);
    int result1;
    int result2;
	result1 = setsockopt(sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq));
    int err1 = GetLastError();
	result2 = setsockopt(sd, IPPROTO_IP, IP_MULTICAST_IF, (char*)&mreq, sizeof(mreq));
    int err2 = GetLastError();
    OutputDebugString(L"gasfgdsj");
}

int UDPSocket::sendtoGroup(char type, void* data, int length)
{
	DWORD Flags;
	LPSOCKET_INFORMATION SocketInfo;
	DWORD SendBytes;
	DWORD WaitResult;
	char* data_send = (char*)malloc(sizeof(char) * (length + 5));

	data_send[0] = type;

	//message len
	data_send[1] = (length >> 24) & 0xFF;
	data_send[2] = (length >> 16) & 0xFF;
	data_send[3] = (length >> 8) & 0xFF;
	data_send[4] = length & 0xFF;

	WaitResult = WaitForSingleObject(mutex, INFINITE);

	if (WaitResult = WAIT_OBJECT_0)
	{
		if ((SocketInfo == (LPSOCKET_INFORMATION)GlobalAlloc(GPTR,
			sizeof(SOCKET_INFORMATION))) == NULL)
		{
			printf("GlobalAlloc() failed with error %d\n", GetLastError());
			return 0;
		}

		SocketInfo->Socket = sd;
		ZeroMemory(&(SocketInfo->Overlapped), sizeof(WSAOVERLAPPED));
		SocketInfo->DataBuf.len = length + 5;
		SocketInfo->DataBuf.buf = data_send;
		Flags = 0;

		if (WSASendTo(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &SendBytes, Flags, (struct sockaddr*)&mreq, sizeof(mreq),
			&(SocketInfo->Overlapped), 0) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				MessageBox(NULL, L"WSASend() failed with error", L"ERROR", MB_ICONERROR);
				return 0;
			}
		}
		else
		{
			return 1;
		}
	}
	else
	{
		MessageBox(NULL, L"Error in the mutex", L"ERROR", MB_ICONERROR);
	}

}
