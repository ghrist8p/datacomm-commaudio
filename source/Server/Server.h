#ifndef SERVER_H
#define SERVER_H

#include <Winsock2.h>
#include <wchar.h>

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
    void startTCP();
    void send( TCPConnection * to
                 , LPWSABUF        lpBuffers
                 , DWORD           dwBufferCount
                 , LPWSAOVERLAPPED lpOverlapped
                 , LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine );
    friend DWORD WINAPI AcceptThread( LPVOID lpParam );
    friend DWORD WINAPI WorkerThread( LPVOID lpParam );
    
    void startUDP();
    void sendToGroup( const char * buf, int len );
private:
    unsigned short tcpPort;
    WSAEVENT newConnectionEvent;
    
    SOCKET listenSocket;
    int numTCPConnections;
    TCPConnection * TCPConnections;
    
    struct sockaddr_in group;
    SOCKET multicastSocket;
};

#endif