#include "Sockets.h"

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: TCPSocket
--
-- DATE: March 17, 2015
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Manuel Gonzales
--
-- PROGRAMMER: Manuel Gonzales
--
-- INTERFACE: TCPSocket::TCPSocket(char* host, int port, MessageQueue mqueue)
--
--  host : ip address to onnect
--	port : port number to connect
--  mqueue : message queue to use for storing the data.
--
--	RETURNS: nothing.
--
--	NOTES:
--  This is the constructor for the TCP socket, it will create the socket and connect to the specified server,
--  then it will start the thread to receive data.
----------------------------------------------------------------------------------------------------------------------*/
TCPSocket::TCPSocket(char* host, int port, MessageQueue* mqueue)
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
		MessageBox(NULL, L"DLL not found- Read Help guide for more information", L"ERROR", MB_ICONERROR);
		return;
	}

	// Create the socket
	if ((sd = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0,
		WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
	{
		MessageBox(NULL, L"Socket cannot be created- Read Help guide for more information", L"ERROR", MB_ICONERROR);
		return;
	}

	// Initialize and set up the address structure
	memset((char *)&server, 0, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);

	if ((hp = gethostbyname(host)) == NULL)
	{
		MessageBox(NULL, L"Unknown server address- Read Help guide for more information", L"ERROR", MB_ICONERROR);
		return;
	}

	// Copy the server address
	memcpy((char *)&server.sin_addr, hp->h_addr, hp->h_length);

	// Connecting to the server
	if (connect(sd, (struct sockaddr *)&server, sizeof(server)) == -1)
	{
		MessageBox(NULL, L"Cannot connect to server- Read Help guide for more information", L"ERROR", MB_ICONERROR);
		return;
	}

	pptr = hp->h_addr_list;

	if ((ThreadHandle = CreateThread(NULL, 0, TCPThread, (void*)this, 0, &ThreadId)) == NULL)
	{
		MessageBox(NULL, L"CreateThread failed with error", L"ERROR", MB_ICONERROR);
		return;
	}
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: TCPThread
--
-- DATE: March 17, 2015
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Manuel Gonzales
--
-- PROGRAMMER: Manuel Gonzales
--
-- INTERFACE: DWORD WINAPI TCPSocket::TCPThread(LPVOID lpParameter)
--
--	lpParameter : instance of the object
--
--	RETURNS: nothing.
--
--	NOTES:
--  This will call the start Thread fucntion
----------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI TCPSocket::TCPThread(LPVOID lpParameter)
{
	TCPSocket* This = (TCPSocket*)lpParameter;
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
-- INTERFACE: DWORD TCPSocket::ThreadStart(void)
--
--	RETURNS: FALSE in error.
--
--	NOTES:
--  This function will start receiving the data from the socket, the first thing it will read would be the data
--  size and then will call the TCPRoutine
----------------------------------------------------------------------------------------------------------------------*/
DWORD TCPSocket::ThreadStart(void)
{
	DWORD Flags;
	LPSOCKET_INFORMATION SocketInfo;	
	DWORD RecvBytes;


		if ((SocketInfo = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR,
			sizeof(SOCKET_INFORMATION))) == NULL)
		{
			MessageBox(NULL, L"GlobalAlloc() failed with error", L"ERROR", MB_ICONERROR);
			return FALSE;
		}

		SocketInfo->Socket = sd;
		ZeroMemory(&(SocketInfo->Overlapped), sizeof(WSAOVERLAPPED));
		SocketInfo->DataBuf.len = GETLENGTH;
		SocketInfo->DataBuf.buf = SocketInfo->Buffer;
		SocketInfo->mqueue = msgqueue;
		Flags = 0;

		while (true)
		{
			if (WSARecv(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &RecvBytes, &Flags,
				0, 0) == SOCKET_ERROR)
			{
                int err;
				if ((err = WSAGetLastError()) != WSA_IO_PENDING)
				{
					MessageBox(NULL, L"WSARecv() failed with error", L"ERROR", MB_ICONERROR);
					return FALSE;
				}
			}		

            
	        int length = (SocketInfo->Buffer[0] << 24) | (SocketInfo->Buffer[1] << 16) | (SocketInfo->Buffer[2] << 8) | (SocketInfo->Buffer[3]);
	        SocketInfo->DataBuf.len = length + 1;

	        if (WSARecv(SocketInfo->Socket, &SocketInfo->DataBuf, 1, &RecvBytes, &Flags,
		        0, 0) == SOCKET_ERROR)
	        {
                int err;
		        if ((err = WSAGetLastError()) != WSA_IO_PENDING)
		        {
			        MessageBox(NULL, L"WSARecv() failed with error", L"ERROR", MB_ICONERROR);
			        return 0;
		        }
	        }
	        else
	        {
		        char type = SocketInfo->Buffer[0];
		        SocketInfo->mqueue->enqueue(type, SocketInfo->Buffer);
	        }
		}
}
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: TCPRoutine
--
-- DATE: March 17, 2015
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Manuel Gonzales
--
-- PROGRAMMER: Manuel Gonzales
--
-- INTERFACE: void CALLBACK TCPSocket::TCPRoutine(DWORD Error, DWORD BytesTransferred,
	LPWSAOVERLAPPED Overlapped, DWORD InFlags)
--
--  Error: Errors when receiving
--  BytesTransferred: Bytes Received
--  Overlapped: Overlapped Structure from the SocketInfo
--  InFlags: Flags for the WSA call
--
--	NOTES:
--  This function will read the data based on the length received earlier and will store it to the message
--  queue based on type.
----------------------------------------------------------------------------------------------------------------------*/
//void CALLBACK TCPSocket::TCPRoutine(DWORD Error, DWORD BytesTransferred,
//	LPWSAOVERLAPPED Overlapped, DWORD InFlags)
//{
//	DWORD Flags;
//	DWORD RecvBytes;
//
//	LPSOCKET_INFORMATION SocketInfo = (LPSOCKET_INFORMATION)Overlapped;
//
//	int length = (SocketInfo->Buffer[3] << 24) | (SocketInfo->Buffer[2] << 16) | (SocketInfo->Buffer[1] << 8) | (SocketInfo->Buffer[0]);
//	SocketInfo->DataBuf.len = length + 1;
//
//	if (WSARecv(SocketInfo->Socket, &SocketInfo->DataBuf, 1, &RecvBytes, &Flags,
//		Overlapped, TCPRoutine) == SOCKET_ERROR)
//	{
//        int err;
//		if ((err = WSAGetLastError()) != WSA_IO_PENDING)
//		{
//			MessageBox(NULL, L"WSARecv() failed with error", L"ERROR", MB_ICONERROR);
//			return;
//		}
//	}
//	else
//	{
//		int type = SocketInfo->Buffer[0];
//		SocketInfo->mqueue->enqueue(type, SocketInfo->Buffer);
//	}
//
//
//}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: ~TCPSocket
--
-- DATE: March 17, 2015
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Manuel Gonzales
--
-- PROGRAMMER: Manuel Gonzales
--
-- INTERFACE: TCPSocket::~TCPSocket()
--
--	RETURNS: nothing.
--
--	NOTES:
--  This is the destructor that will do the cleanup.
----------------------------------------------------------------------------------------------------------------------*/
TCPSocket::~TCPSocket()
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
-- INTERFACE: int TCPSocket::Send(void* data, int length)
--
--	data : data to send
--  length : data length
--
--	RETURNS: 1 in sucess, 0 in error
--
--	NOTES:
--  This will send the desired data to the server.
----------------------------------------------------------------------------------------------------------------------*/
int TCPSocket::Send(char type, void* data, int length)
{
	DWORD Flags;
	LPSOCKET_INFORMATION SocketInfo;
	DWORD RecvBytes;
	DWORD WaitResult;
	char* data_send = (char*) malloc(sizeof(char) * (length + 5));
	
	data_send[0] = type;

	//message len
	data_send[1] = (length >> 24) & 0xFF;
	data_send[2] = (length >> 16) & 0xFF;
	data_send[3] = (length >> 8) & 0xFF;
	data_send[4] = length & 0xFF;

	memcpy(data_send + 5, (char*)data, length);

	WaitResult = WaitForSingleObject( mutex, INFINITE);

	if (WaitResult == WAIT_OBJECT_0)
	{
		if ((SocketInfo = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR,
			sizeof(SOCKET_INFORMATION))) == NULL)
		{
			MessageBox(NULL, L"GlobalAlloc() failed with error", L"ERROR", MB_ICONERROR);
			return 0;
		}

		SocketInfo->Socket = sd;
		ZeroMemory(&(SocketInfo->Overlapped), sizeof(WSAOVERLAPPED));
		SocketInfo->DataBuf.len = length + 5;
		SocketInfo->DataBuf.buf = data_send;
		Flags = 0;

		if (WSASend(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &RecvBytes, Flags,
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
        int err = GetLastError();
		return 0;
	}

}

