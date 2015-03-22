#ifndef SERVER_H
#define SERVER_H

#include <windows.h>
#include <WinSock2.h>

class Server
{
public:
    Server( unsigned short _tcpPort, unsigned short _udpPort );
    virtual ~Server();
    void start();
    friend DWORD WINAPI WorkerThread( LPVOID lpParam );
private:
    unsigned short tcpPort;
    unsigned short udpPort;
    WSAEVENT newConnectionEvent;
    SOCKET newConnection;
};

#endif