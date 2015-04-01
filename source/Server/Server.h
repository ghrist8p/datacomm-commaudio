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
    void submitCompletionRoutine( PAPCFUNC lpCompletionRoutine, TCPConnection * to );
    
    void startUDP();
    void sendToGroup( const char * buf, int len );
private:
    unsigned short tcpPort;
    SOCKET listenSocket;
    
    WSAEVENT newConnectionEvent;
    
    int numTCPConnections;
    TCPConnection * TCPConnections;
    
    static DWORD WINAPI AcceptThread( LPVOID lpParam );
    HANDLE hAcceptThread;
    static DWORD WINAPI WorkerThread( LPVOID lpParam );
    HANDLE hWorkerThread;
    
    
    struct sockaddr_in group;
    SOCKET multicastSocket;
};

#endif