#include "Sockets.h"
#include "../Buffer/MessageQueue.h"

using namespace std;
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
	: msgqueue(mqueue)
{
	int error;
	struct sockaddr_in server;
	WSADATA WSAData;
	WORD wVersionRequested;

	HANDLE ThreadHandle;
	DWORD ThreadId;
	stopSending = false;

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
	server.sin_port = htons(MULTICAST_PORT);
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	// Copy the server address

	// Connecting to the server
    char reuseAddr = 1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &reuseAddr, sizeof(reuseAddr));
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
    setsockopt(sd, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char*)&mreq, sizeof(mreq));
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
	SOCKET_INFORMATION socketInfo;
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

	if (WaitResult == WAIT_OBJECT_0)
	{

		socketInfo.Socket = sd;
		ZeroMemory(&(socketInfo.Overlapped), sizeof(WSAOVERLAPPED));
		socketInfo.DataBuf.len = length + 5;
		socketInfo.DataBuf.buf = data_send;
		Flags = 0;

		memset(&destination,0,destsize);

		destination.sin_family = AF_INET;
		destination.sin_addr.s_addr = inet_addr(dest_ip);
		if (destination.sin_addr.s_addr == INADDR_NONE)
		{
			MessageBox(NULL, L"The target ip address entered must be a legal IPv4 address", L"ERROR", MB_ICONERROR);
			return 0;
		}

		destination.sin_port = htons(dest_port);

		if (destination.sin_port == 0)
		{
			MessageBox(NULL, L"The targetport must be a legal UDP port number", L"ERROR", MB_ICONERROR);
			return 0;
		}

		if (WSASendTo(socketInfo.Socket, &(socketInfo.DataBuf), 1, &SendBytes, Flags, (SOCKADDR*)&destination, destsize,
			0, 0) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				int err = GetLastError();
				MessageBox(NULL, L"WSASend() failed with error", L"ERROR", MB_ICONERROR);
				return 0;
			}
		}
		else
		{
			free(data_send);
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
	SOCKET_INFORMATION socketInfo;
	DWORD RecvBytes;
	int flag = 0;
	int msg_type = 0;
	int len = 0;
	struct sockaddr_in source;
	int length = sizeof(struct sockaddr_in);

	socketInfo.Socket = sd;
	ZeroMemory(&(socketInfo.Overlapped), sizeof(WSAOVERLAPPED));
	socketInfo.DataBuf.buf = socketInfo.Buffer;
	socketInfo.mqueue = msgqueue;
	Flags = 0;

	while (true)
	{
		socketInfo.DataBuf.len = DATA_BUFSIZE;

		if (WSARecvFrom(socketInfo.Socket, &(socketInfo.DataBuf), 1, &RecvBytes, &Flags, (sockaddr*)&source,
			&length, 0, 0) == SOCKET_ERROR)
		{
			int err;
			if ((err = WSAGetLastError()) != WSA_IO_PENDING)
			{
				MessageBox(NULL, L"WSARecv() failed with error", L"ERROR", MB_ICONERROR);
				return FALSE;
			}
		}
		else
		{
			len = RecvBytes - 1;

			DataPacket dataPacket;
			LocalDataPacket localDataPacket;
			memcpy(&dataPacket,socketInfo.Buffer+1,len);
			localDataPacket.index = dataPacket.index;
			localDataPacket.srcAddr = source.sin_addr.s_addr;
			memcpy(localDataPacket.data,dataPacket.data,DATA_LEN);
			socketInfo.mqueue->enqueue(socketInfo.Buffer[0],&localDataPacket,sizeof(LocalDataPacket));
		}
	}
}

void UDPSocket::setGroup(char* group_address, int mem_flag)
{
    char loop = 0;
    char ttl = 2;
    in_addr interfaceAddr;
    interfaceAddr.s_addr = inet_addr(INADDR_ANY);
	memset(&mreq,0,sizeof(mreq));
	mreq.imr_multiaddr.s_addr = inet_addr(group_address);
    mreq.imr_interface.s_addr = inet_addr(INADDR_ANY);
    int i = 0;
    int err = 0;
	if (mem_flag)
	{
		i = setsockopt(sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq));
        err = GetLastError();
        i = setsockopt(sd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));
        err = GetLastError();
        i = setsockopt(sd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));
        err = GetLastError();
	}
	setsockopt(sd, IPPROTO_IP, IP_MULTICAST_IF, (char*)&interfaceAddr, sizeof(interfaceAddr));
}

int UDPSocket::sendtoGroup(char type, void* data, int length)
{
	DWORD Flags;
	SOCKET_INFORMATION socketInfo;
	DWORD SendBytes;
	DWORD WaitResult;
	char* data_send = (char*)malloc(sizeof(char) * (length + 1));

	data_send[0] = type;

    DataPacket* p = (DataPacket*) data;

    memcpy(data_send + 1, (char*)data, length);

	WaitResult = WaitForSingleObject(mutex, INFINITE);

	if (WaitResult == WAIT_OBJECT_0)
	{

		socketInfo.Socket = sd;
		ZeroMemory(&(socketInfo.Overlapped), sizeof(WSAOVERLAPPED));
		socketInfo.DataBuf.len = length + 1;
		socketInfo.DataBuf.buf = data_send;
		Flags = 0;

		sockaddr_in address;
		memset(&address,0,sizeof(address));
		address.sin_family = AF_INET;
		address.sin_port   = htons(MULTICAST_PORT);
		memcpy(&address.sin_addr,&mreq.imr_multiaddr,sizeof(struct in_addr));

		if (WSASendTo(socketInfo.Socket, &(socketInfo.DataBuf), 1, &SendBytes, Flags, (struct sockaddr*)&address, sizeof(address),
			0, 0) == SOCKET_ERROR)
		{
			int err;
			if ((err = WSAGetLastError()) != WSA_IO_PENDING)
			{
				MessageBox(NULL, L"WSASend() failed with error", L"ERROR", MB_ICONERROR);
				return 0;
			}
		}
		else
		{
			free(data_send);
			return 1;
		}
	}
	else
	{
		MessageBox(NULL, L"Error in the mutex", L"ERROR", MB_ICONERROR);
	}


}

MessageQueue* UDPSocket::getMessageQueue()
{
    return msgqueue;
}

void UDPSocket::sendWave(SongName songloc, int speed, vector<TCPSocket*> sockets)
{
	FILE* fp = fopen(songloc.filepath, "rb");
	struct SongStream songInfo;
	char* sendSong;
	char* song;
	stopSending = false;

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

				sendSong = (char*)malloc(sizeof(char) * SIZE_INDEX);

				sendSong[0] = (songloc.id >> 24) & 0xFF;
				sendSong[1] = (songloc.id >> 16) & 0xFF;
				sendSong[2] = (songloc.id >> 8) & 0xFF;
				sendSong[3] = songloc.id & 0xFF;

				//for every client
				for (int i = 0; i < sockets.size(); i++)
				{
					sockets[i]->Send(CHANGE_STREAM, sendSong, SIZE_INDEX);
				}

				free(sendSong);

				song = (char*)malloc(speed + 5);

				//read chunks of data from the file based on the speed selected and send it
				while (data_read = fread(song + 5, 1, speed, fp) > 0)
				{
					if (stopSending)
					{
						return;
					}

					sendtoGroup(MUSICSTREAM, song, data_read);
				}

				free(song);

				stopSending = false;
			}
			else
			{
				MessageBox(NULL, L"NOT WAVE", L"ERROR", MB_ICONERROR);
			}
		}
		else
		{
			MessageBox(NULL, L"NOT RIFF", L"ERROR", MB_ICONERROR);
		}
	}

	fclose(fp);
}

void UDPSocket::stopSong()
{
	stopSending = true;
}
