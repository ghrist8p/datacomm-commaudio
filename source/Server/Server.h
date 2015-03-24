#ifndef SERVER_H
#define SERVER_H

#include <WinSock2.h>

typedef struct _TCPConnection
{
    SOCKET sock;
    // add more connection data at will
} TCPConnection;

class Server
{
public:
    Server( unsigned short _tcpPort, unsigned long groupIP, unsigned short udpPort );
    virtual ~Server();
    void startTCP(); // CAUTION infinate loop!
    void startUDP();
    void sendToGroup( const char * buf, int len );
    friend DWORD WINAPI WorkerThread( LPVOID lpParam );
private:
    unsigned short tcpPort;
    WSAEVENT newConnectionEvent;
    
    int numTCPConnections;
    TCPConnection * TCPConnections;
    
    struct sockaddr_in group;
    SOCKET multicastSocket;
};

#endif