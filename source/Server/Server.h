#ifndef SERVER_H
#define SERVER_H

#include <ws2tcpip.h>
#include <Winsock2.h>
#include <wchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
using namespace std;

struct WavSong
{
	char* data;
	unsigned long size;
};


typedef struct _TCPConnection
{
	SOCKET sock;
	// add more connection data at will
} TCPConnection;

class Server
{
public:
	Server(unsigned short _tcpPort, unsigned long groupIP, unsigned short udpPort);
	virtual ~Server();
	void startTCP();
	void submitCompletionRoutine(PAPCFUNC lpCompletionRoutine, TCPConnection * to);

	void startUDP();
	void sendToGroup(const char * buf, int len);
	void sendWave(char* fname, WavSong *ret, int speed);

private:
	unsigned short tcpPort;
	SOCKET listenSocket;

	WSAEVENT newConnectionEvent;

	int numTCPConnections;
	TCPConnection * TCPConnections;

	static DWORD WINAPI AcceptThread(LPVOID lpParam);
	HANDLE hAcceptThread;
	static DWORD WINAPI WorkerThread(LPVOID lpParam);
	HANDLE hWorkerThread;


	struct sockaddr_in group;
	SOCKET multicastSocket;
};

#endif