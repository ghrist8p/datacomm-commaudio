#include <winsock2.h>
#include <windows.h>
#include <stdio.h>

#define GETLENGTH	4
#define DATA_BUFSIZE 8192

typedef struct _SOCKET_INFORMATION {
	OVERLAPPED Overlapped;
	SOCKET Socket;
	CHAR Buffer[DATA_BUFSIZE];
	WSABUF DataBuf;
	MessageQueue mqueue;
} SOCKET_INFORMATION, *LPSOCKET_INFORMATION;

class TCPSocket
{
private:
	SOCKET sd;
	MessageQueue msgqueue;
	HANDLE mutex;

public:
	TCPSocket(char* host, int port, MessageQueue mqueue);
	~TCPSocket();
	int Send(void* data, int lenght);
	static DWORD WINAPI TCPThread(LPVOID lpParameter);
	DWORD ThreadStart(void);
	static void CALLBACK TCPRoutine(DWORD Error, DWORD BytesTransferred,
		LPWSAOVERLAPPED Overlapped, DWORD InFlags);

};
