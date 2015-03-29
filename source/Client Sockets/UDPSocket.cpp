#include "Sockets.h"

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
	struct hostent	*hp = nullptr;
	struct sockaddr_in server;
	char** pptr;
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
		printf("DLL not fount- Read Help guide for more information");
		return;
	}

	// Create the socket
	if ((sd = WSASocket(PF_INET, SOCK_DGRAM, 0, NULL, 0,
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

	pptr = hp->h_addr_list;

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
int UDPSocket::Send(void* data, int length, char* dest_ip, int dest_port)
{
	DWORD Flags;
	LPSOCKET_INFORMATION SocketInfo;
	DWORD SendBytes;
	DWORD WaitResult;
	struct sockaddr_in destination;
	int destsize = sizeof(destination);

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
		SocketInfo->DataBuf.len = length;
		SocketInfo->DataBuf.buf = (char*)data;
		Flags = 0;

		destination.sin_addr.s_addr = inet_addr(dest_ip);
		if (destination.sin_addr.s_addr == INADDR_NONE) 
		{
			printf("The target ip address entered must be a legal IPv4 address\n");
			return 0;
		}

		destination.sin_port = htons((u_short)dest_port);

		if (destination.sin_port == 0) 
		{
			printf("The targetport must be a legal UDP port number\n");
			return 0;
		}

		if (WSASendTo(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &SendBytes, Flags, (SOCKADDR*)&destination, destsize,
			&(SocketInfo->Overlapped), 0) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				printf("WSASend() failed with error %d\n", WSAGetLastError());
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
		printf("Error in the mutex");
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


	if ((SocketInfo = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR,
		sizeof(SOCKET_INFORMATION))) == NULL)
	{
		printf("GlobalAlloc() failed with error %d\n", GetLastError());
		return FALSE;
	}

	SocketInfo->Socket = sd;
	ZeroMemory(&(SocketInfo->Overlapped), sizeof(WSAOVERLAPPED));
	SocketInfo->DataBuf.len = DATA_BUFSIZE;
	SocketInfo->DataBuf.buf = SocketInfo->Buffer;
	SocketInfo->mqueue = msgqueue;
	Flags = 0;

	while (true)
	{
		if (WSARecvFrom(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &RecvBytes, &Flags, 0,
			0, &(SocketInfo->Overlapped), 0) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				printf("WSARecv() failed with error %d\n", WSAGetLastError());
				return FALSE;
			}
		}
		else
		{
			int type = SocketInfo->Buffer[0];
			SocketInfo->mqueue->enqueue(type, SocketInfo->Buffer);
		}
	}
}/*

void CALLBACK UDPSocket::UDPRoutine(DWORD Error, DWORD BytesTransferred,
	LPWSAOVERLAPPED Overlapped, DWORD InFlags)
{
	DWORD Flags;
	DWORD RecvBytes;

	LPSOCKET_INFORMATION SocketInfo = (LPSOCKET_INFORMATION)Overlapped;

	if ((WSARecvFrom(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &RecvBytes, &Flags, 0,
		0, &(SocketInfo->Overlapped), UDPRoutine) == SOCKET_ERROR))
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			printf("WSARecv() failed with error %d\n", WSAGetLastError());
			return;
		}
	}
	else
	{
		int type = SocketInfo->Buffer[0];
		SocketInfo->mqueue.enqueue(type, SocketInfo->Buffer);
	}


}*/

