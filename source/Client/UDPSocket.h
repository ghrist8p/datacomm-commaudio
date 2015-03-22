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

class UDPSocket
{
private:
	SOCKET sd;
	MessageQueue msgqueue;
	HANDLE mutex;

public:
	UDPSocket(int port, MessageQueue mqueue);
	~UDPSocket();
	int Send(void* data, int length, char* dest_ip, int dest_port);
	static DWORD WINAPI UDPThread(LPVOID lpParameter);
	DWORD ThreadStart(void);
	static void CALLBACK UDPRoutine(DWORD Error, DWORD BytesTransferred,
		LPWSAOVERLAPPED Overlapped, DWORD InFlags);

};