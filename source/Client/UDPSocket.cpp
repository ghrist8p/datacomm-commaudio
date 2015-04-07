/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: UDPSocket.cpp
--
-- FUNCTIONS:
	UDPSocket(int port, MessageQueue* mqueue);
	~UDPSocket();
	DWORD ThreadStart(void);
	static void CALLBACK UDPRoutine(DWORD Error, DWORD BytesTransferred,
	LPWSAOVERLAPPED Overlapped, DWORD InFlags);
	static DWORD WINAPI UDPThread(LPVOID lpParameter);
	int Send(char type, void* data, int length, char* dest_ip, int dest_port);
	int sendtoGroup(char type, void* data, int length);
	void setGroup(char* group_address, int mem_flag);
	MessageQueue* getMessageQueue();
	void stopSong();
	void sendWave(SongName songloc, int speed, std::vector<TCPSocket*> sockets);
--
-- DATE: April 1, 2015
--
-- REVISIONS: April 4, 2015		Eric Tsang
--			Fixed Memory leaks and buffer size problems.
--
-- DESIGNER: Manuel Gonzales
--
-- PROGRAMMER: Manuel Gonzales
--
-- NOTES:
-- This is the file containing all the necessary functions for the UDP Socket to send and receive following the protocol
-- created. This class is used for the server as well as the client socket.
----------------------------------------------------------------------------------------------------------------------*/

#include "Sockets.h"
#include "../Buffer/MessageQueue.h"
#include "../Server/ServerControlThread.h"

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
		MessageBox(NULL, L"WSA error", L"ERROR", MB_ICONERROR);
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
		OutputDebugString(L"Can't bind name to socket");
		exit(1);
	}

	if ((ThreadHandle = CreateThread(NULL, 0, UDPThread, (void*)this, 0, &ThreadId)) == NULL)
	{
		OutputDebugString(L"CreateThread failed with error %d\n");
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
-- REVISIONS: April 4, 2015 Manuel Gonzales Added type.
--
-- DESIGNER: Manuel Gonzales
--
-- PROGRAMMER: Manuel Gonzales
--
-- INTERFACE: int UDPSocket::Send(char type, void* data, int length, char* dest_ip, int dest_port)
--
--  type : type of data
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
	char* data_send = (char*)malloc(sizeof(char) * (length + 1));

	data_send[0] = type;

	memcpy(data_send + 1, (char*)data, length);

	WaitResult = WaitForSingleObject(mutex, INFINITE);

	if (WaitResult == WAIT_OBJECT_0)
	{

		socketInfo.Socket = sd;
		ZeroMemory(&(socketInfo.Overlapped), sizeof(WSAOVERLAPPED));
		socketInfo.DataBuf.len = length + 1;
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
			ReleaseMutex(mutex);
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: setGroup
--
-- DATE: April 2, 2015
--
-- REVISIONS: April 4  Eric Tsang
--
-- DESIGNER: Manuel Gonzales
--
-- PROGRAMMER: Manuel Gonzales
--
-- INTERFACE: void UDPSocket::setGroup(char* group_address, int mem_flag)
--
--	group_address : ip address for the multicast
--  mem_flag : add socket to the group flag
--
--	RETURNS: nothing.
--
--	NOTES:
--  This function will set the multicast flag for the udp socket and will add the socket to the gorup as well
--	depending on the type of socket.
----------------------------------------------------------------------------------------------------------------------*/
void UDPSocket::setGroup(char* group_address, int mem_flag)
{
	memset(&mreq,0,sizeof(mreq));
	mreq.imr_multiaddr.s_addr = inet_addr(group_address);
	mreq.imr_interface.s_addr = inet_addr(INADDR_ANY);

	char loop;
	char ttl = 2;

    in_addr interfaceAddr;
	interfaceAddr.s_addr = inet_addr(INADDR_ANY);

    if (mem_flag)
	{
        loop = 0;
	}
    else
    {
        loop = 1;
    }

    setsockopt(sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq));
    setsockopt(sd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));
    setsockopt(sd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));
	setsockopt(sd, IPPROTO_IP, IP_MULTICAST_IF, (char*)&interfaceAddr, sizeof(interfaceAddr));
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: sendtoGroup
--
-- DATE: April 2, 2015
--
-- REVISIONS: April 4  Eric Tsang
--
-- DESIGNER: Manuel Gonzales
--
-- PROGRAMMER: Manuel Gonzales
--
-- INTERFACE: int UDPSocket::sendtoGroup(char type, void* data, int length)
--
--	type : type of data to send
--  data : data to send
--	length : size of data in bytes
--
--	RETURNS: nothing.
--
--	NOTES:
--  This function will send a datagram via multicast to the default group for the socket
----------------------------------------------------------------------------------------------------------------------*/
int UDPSocket::sendtoGroup(char type, void* data, int length)
{
	DWORD Flags;
	SOCKET_INFORMATION socketInfo;
	DWORD SendBytes;
	DWORD WaitResult;
	char* data_send = (char*)malloc(sizeof(char) * (length + 1));

	data_send[0] = type;

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

		//sockaddr_in address;
		//memset(&address,0,sizeof(address));
		//address.sin_family      = AF_INET;
		//address.sin_port        = htons(MULTICAST_PORT);
		//address.sin_addr.s_addr = inet_addr("192.168.0.20");

		if (WSASendTo(socketInfo.Socket, &(socketInfo.DataBuf), 1, &SendBytes, Flags, (struct sockaddr*)&address, sizeof(address),
			0, 0) == SOCKET_ERROR)
		{
			int err;
			if ((err = WSAGetLastError()) != WSA_IO_PENDING)
			{
				wchar_t errorStr[256] = {0};
				swprintf_s( errorStr, 256, L"WSASend() failed with error: %d", err );
				MessageBox(NULL, errorStr, L"Error", MB_ICONERROR);
				return 0;
			}
		}
		else
		{
			free(data_send);
			ReleaseMutex(mutex);
			return 1;
		}
	}
	else
	{
		MessageBox(NULL, L"Error in the mutex", L"ERROR", MB_ICONERROR);
	}


}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: getMessageQueue
--
-- DATE: April 2, 2015
--
-- REVISIONS: --
--
-- DESIGNER: Eric Tsang
--
-- PROGRAMMER: Eric Tsang
--
-- INTERFACE: MessageQueue* UDPSocket::getMessageQueue()
--
--	RETURNS: message queue pointer.
--
--	NOTES:
--  This function will return a pointer to the message queue being used by the socket
----------------------------------------------------------------------------------------------------------------------*/
MessageQueue* UDPSocket::getMessageQueue()
{
	return msgqueue;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: sendWave
--
-- DATE: April 2, 2015
--
-- REVISIONS: --
--
-- DESIGNER: Manuel Gonzales
--
-- PROGRAMMER: Manuel Gonzales
--
-- INTERFACE: void UDPSocket::sendWave(SongName songloc, int speed, vector<TCPSocket*> sockets)
--
--	songloc : structure golding the id and path of the song
--  speed : sending speed in bytes
--	sockets : a set of TCP sockets (clients)
--
--	RETURNS: nothing.
--
--	NOTES:
--  This function will send a song via multicast to all the clients and will send it in parts bnased on the speed
--	it will also send a flag notifying the clients the current song that is being played.
----------------------------------------------------------------------------------------------------------------------*/
void UDPSocket::sendWave(SongName songloc, int speed, vector<TCPSocket*> sockets)
{
	ServerControlThread * sct = ServerControlThread::getInstance();
	wchar_t * path = sct->getPlaylist()->getSongPath( songloc.id );
    char mbspath[STR_LEN];
    sprintf(mbspath,"%S",path);
	FILE* fp = fopen(mbspath, "rb");

	free( path );

	struct SongStream songInfo;
	char* sendSong;
	char* song;
	stopSending = false;

	if (fp)
	{
		RequestPacket packet;

		packet.index = songloc.id;

		//for every client
		for (int i = 0; i < sockets.size(); i++)
		{
			sockets[i]->Send(CHANGE_STREAM, &packet, sizeof(packet));
		}

		// continuously send voice data over the network when it becomes
		// available
		DataPacket voicePacket;
		int count = 0;
		voicePacket.index = 0;
		char sound[DATA_LEN];
		while(fread(sound,1,DATA_LEN,fp))
		{
			if(stopSending)
			{
				break;
			}
			++(voicePacket.index);
			memcpy(voicePacket.data, sound, DATA_LEN);
			sendtoGroup(MUSICSTREAM,&voicePacket,sizeof(voicePacket));
			if(count > 3)
			{
				count = 0;
				Sleep(1);
			}
		}
		fclose(fp);
	}


	// struct SongStream songInfo;
	// char* sendSong;
	// char* song;
	// stopSending = false;

	// if ( hSong == INVALID_HANDLE_VALUE )
	// {
 //        wchar_t errorStr[256] = {0};
 //        swprintf( errorStr, 256, L"CreateFile() failed: %d", GetLastError() );
 //        MessageBox(NULL, errorStr, L"Error", MB_ICONERROR);
	// 	return;
	// }
	// else
	// {

	// 	char id[5];
	// 	unsigned long size;
	// 	short format_tag, channels, block_align, bits_per_sample;
	// 	unsigned long format_length, sample_rate, avg_bytes_sec, data_size;
	// 	int data_read = 0;

	// 	ReadFile( hSong, id, sizeof( char ) * 4, NULL, NULL );
	// 	id[4] = '\0';

	// 	if (!strcmp(id, "RIFF")) {
	// 		ReadFile( hSong, &size, sizeof(unsigned long) * 1, NULL, NULL );
	// 		ReadFile( hSong, id, sizeof(char) * 4, NULL, NULL );
	// 		id[4] = '\0';

	// 		if (!strcmp(id, "WAVE")) {
	// 			//get wave headers
	// 			ReadFile( hSong, id, sizeof(char) * 4, NULL, NULL );
	// 			ReadFile( hSong, &format_length, sizeof(unsigned long) * 1, NULL, NULL );
	// 			ReadFile( hSong, &format_tag, sizeof(short) * 1, NULL, NULL );
	// 			ReadFile( hSong, &channels, sizeof(short) * 1, NULL, NULL );
	// 			ReadFile( hSong, &sample_rate, sizeof(unsigned long) * 1, NULL, NULL );
	// 			ReadFile( hSong, &avg_bytes_sec, sizeof(unsigned long) * 1, NULL, NULL );
	// 			ReadFile( hSong, &block_align, sizeof(short) * 1, NULL, NULL );
	// 			ReadFile( hSong, &bits_per_sample, sizeof(short) * 1, NULL, NULL );
	// 			ReadFile( hSong, id, sizeof(char) * 4, NULL, NULL );
	// 			ReadFile( hSong, &data_size, sizeof(unsigned long) * 1, NULL, NULL );

	// 			sendSong = (char*)malloc(sizeof(char) * SIZE_INDEX);

	// 			sendSong[0] = (songloc.id >> 24) & 0xFF;
	// 			sendSong[1] = (songloc.id >> 16) & 0xFF;
	// 			sendSong[2] = (songloc.id >> 8) & 0xFF;
	// 			sendSong[3] = songloc.id & 0xFF;

	// 			//for every client
	// 			for (int i = 0; i < sockets.size(); i++)
	// 			{
	// 				sockets[i]->Send(CHANGE_STREAM, sendSong, SIZE_INDEX);
	// 			}

	// 			free(sendSong);

	// 			song = (char*)malloc(speed);

	// 			//read chunks of data from the file based on the speed selected and send it
	// 			while (data_read = ReadFile( hSong, song, speed, NULL, NULL ) > 0)
	// 			{
	// 				if (stopSending)
	// 				{
	// 					return;
	// 				}

	// 				sendtoGroup(MUSICSTREAM, song, data_read);
	// 			}

	// 			free(song);

	// 			stopSending = false;
	// 		}
	// 		else
	// 		{
	// 			MessageBox(NULL, L"NOT WAVE", L"ERROR", MB_ICONERROR);
	// 		}
	// 	}
	// 	else
	// 	{
	// 		MessageBox(NULL, L"NOT RIFF", L"ERROR", MB_ICONERROR);
	// 	}
	// }

	// CloseHandle( hSong );
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: stopSong
--
-- DATE: April 2, 2015
--
-- REVISIONS: --
--
-- DESIGNER: Manuel Gonzales
--
-- PROGRAMMER: Manuel Gonzales
--
-- INTERFACE: void UDPSocket::stopSong()
--
--	RETURNS: nothing.
--
--	NOTES:
--  This function will set the flag to stop sending music in the case of a change of song event
----------------------------------------------------------------------------------------------------------------------*/
void UDPSocket::stopSong()
{
	stopSending = true;
}
