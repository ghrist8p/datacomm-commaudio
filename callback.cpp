// Module Name: callback.cpp
//
// Description:
//
//     This sample illustrates how to develop a simple echo server Winsock
//     application using the Overlapped I/O model with callback routines.
//     This sample is implemented as a console-style application and simply prints
//     messages when connections are established and removed from the server.
//     The application listens for TCP connections on port 5150 and accepts them
//     as they arrive. When this application receives data from a client, it
//     simply echos (this is why we call it an echo server) the data back in
//     it's original form until the client closes the connection.
//
// Compile:
//
//     cl -o callback callback.cpp ws2_32.lib
//
// Command Line Options:
//
//     callback.exe
//
//     Note: There are no command line options for this sample.

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>

#define PORT 5150
#define DATA_BUFSIZE 8192

typedef struct _SOCKET_INFORMATION {
    OVERLAPPED Overlapped;
    SOCKET Socket;
    CHAR Buffer[DATA_BUFSIZE];
    WSABUF DataBuf;
    DWORD BytesSEND;
    DWORD BytesRECV;
} SOCKET_INFORMATION, * LPSOCKET_INFORMATION;

void CALLBACK WorkerRoutine(DWORD Error, DWORD BytesTransferred,
    LPWSAOVERLAPPED Overlapped, DWORD InFlags);

DWORD WINAPI WorkerThread(LPVOID lpParameter);

SOCKET AcceptSocket;

void main(void)
{
    WSADATA wsaData;
    SOCKET ListenSocket;
    SOCKADDR_IN InternetAddr;
    INT Ret;
    HANDLE ThreadHandle;
    DWORD ThreadId;
    WSAEVENT AcceptEvent;

    if ((Ret = WSAStartup(0x0202,&wsaData)) != 0)
    {
        printf("WSAStartup failed with error %d\n", Ret);
        WSACleanup();
        return;
    }

    // create listening socket
    if ((ListenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0,
        WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
    {
        printf("Failed to get a socket %d\n", WSAGetLastError());
        return;
    }

    // initialize local address structure
    InternetAddr.sin_family = AF_INET;
    InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    InternetAddr.sin_port = htons(PORT);

    // bind the listening socket to our local address
    if (bind(ListenSocket, (PSOCKADDR) &InternetAddr,
        sizeof(InternetAddr)) == SOCKET_ERROR)
    {
        printf("bind() failed with error %d\n", WSAGetLastError());
        return;
    }

    // put our socket into listening mode
    if (listen(ListenSocket, 5))
    {
        printf("listen() failed with error %d\n", WSAGetLastError());
        return;
    }

    // create the accept event. it gets signaled whenever a new connection is
    // accepted
    if ((AcceptEvent = WSACreateEvent()) == WSA_INVALID_EVENT)
    {
        printf("WSACreateEvent() failed with error %d\n", WSAGetLastError());
        return;
    }

    // create a worker thread to service completed I/O requests.
    if ((ThreadHandle = CreateThread(NULL, 0, WorkerThread, (LPVOID) AcceptEvent, 0, &ThreadId)) == NULL)
    {
        printf("CreateThread failed with error %d\n", GetLastError());
        return;
    }

    while(TRUE)
    {
        AcceptSocket = accept(ListenSocket, NULL, NULL);

        if (WSASetEvent(AcceptEvent) == FALSE)
        {
            printf("WSASetEvent failed with error %d\n", WSAGetLastError());
            return;
        }
    }
}

/**
 * accepts connections, and then performs an asynchronous receive on each one,
 *   and making them invoke WorkerRoutine as a callback when something is
 *   actually received.
 *
 * @param      lpParameter parameter for this thread; handle to event that is set when accepts occur.
 *
 * @return     false if an error has occurred; true otherwise.
 */
DWORD WINAPI WorkerThread(LPVOID lpParameter)
{
    DWORD Flags;
    LPSOCKET_INFORMATION SocketInfo;
    WSAEVENT EventArray[1];
    DWORD Index;
    DWORD RecvBytes;

    // save the accept event in the event array.
    EventArray[0] = (WSAEVENT) lpParameter;

    while(TRUE)
    {
        // wait for accept() to signal an event and also process WorkerRoutine()
        // returns.
        while(TRUE)
        {
            Index = WSAWaitForMultipleEvents(1, EventArray, FALSE, WSA_INFINITE, TRUE);

            if (Index == WSA_WAIT_FAILED)
            {
                printf("WSAWaitForMultipleEvents failed with error %d\n", WSAGetLastError());
                return FALSE;
            }

            if (Index != WAIT_IO_COMPLETION)
            {
                // an accept() call event is ready - break the wait loop
                break;
            }
        }

        WSAResetEvent(EventArray[Index - WSA_WAIT_EVENT_0]);

        // create a socket information structure to associate with the accepted
        // socket.
        if ((SocketInfo = (LPSOCKET_INFORMATION) GlobalAlloc(GPTR,
            sizeof(SOCKET_INFORMATION))) == NULL)
        {
            printf("GlobalAlloc() failed with error %d\n", GetLastError());
            return FALSE;
        }

        // fill in the details of our accepted socket.
        SocketInfo->Socket = AcceptSocket;
        ZeroMemory(&(SocketInfo->Overlapped), sizeof(WSAOVERLAPPED));
        SocketInfo->BytesSEND = 0;
        SocketInfo->BytesRECV = 0;
        SocketInfo->DataBuf.len = DATA_BUFSIZE;
        SocketInfo->DataBuf.buf = SocketInfo->Buffer;

        Flags = 0;
        if (WSARecv(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &RecvBytes, &Flags,
            &(SocketInfo->Overlapped), WorkerRoutine) == SOCKET_ERROR)
        {
            if (WSAGetLastError() != WSA_IO_PENDING)
            {
                printf("WSARecv() failed with error %d\n", WSAGetLastError());
                return FALSE;
            }
        }

        printf("Socket %d connected\n", AcceptSocket);
    }

    return TRUE;
}

void CALLBACK WorkerRoutine(DWORD Error, DWORD BytesTransferred,
    LPWSAOVERLAPPED Overlapped, DWORD InFlags)
{
    DWORD SendBytes, RecvBytes;
    DWORD Flags;

    // reference the WSAOVERLAPPED structure as a SOCKET_INFORMATION structure
    LPSOCKET_INFORMATION SI = (LPSOCKET_INFORMATION) Overlapped;

    if (Error != 0)
    {
        printf("I/O operation failed with error %d\n", Error);
    }

    if (BytesTransferred == 0)
    {
        printf("Closing socket %d\n", SI->Socket);
    }

    if (Error != 0 || BytesTransferred == 0)
    {
        closesocket(SI->Socket);
        GlobalFree(SI);
        return;
    }

    // check to see if the BytesRECV field equals zero. If this is so, then this
    // means a WSARecv call just completed so update the BytesRECV field with
    // the BytesTransferred value from the completed WSARecv() call.
    if (SI->BytesRECV == 0)
    {
        SI->BytesRECV = BytesTransferred;
        SI->BytesSEND = 0;
    }
    else
    {
        SI->BytesSEND += BytesTransferred;
    }

    if (SI->BytesRECV > SI->BytesSEND)
    {

        // post another WSASend() request. Since WSASend() is not gauranteed to
        // send all of the bytes requested, continue posting WSASend() calls
        // until all received bytes are sent.
        ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));

        SI->DataBuf.buf = SI->Buffer + SI->BytesSEND;
        SI->DataBuf.len = SI->BytesRECV - SI->BytesSEND;

        if (WSASend(SI->Socket, &(SI->DataBuf), 1, &SendBytes, 0,
            &(SI->Overlapped), WorkerRoutine) == SOCKET_ERROR)
        {
            if (WSAGetLastError() != WSA_IO_PENDING)
            {
                printf("WSASend() failed with error %d\n", WSAGetLastError());
                return;
            }
        }
    }
    else
    {
        SI->BytesRECV = 0;

        // now that there are no more bytes to send post another WSARecv()
        // request.
        Flags = 0;
        ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));

        SI->DataBuf.len = DATA_BUFSIZE;
        SI->DataBuf.buf = SI->Buffer;

        if (WSARecv(SI->Socket, &(SI->DataBuf), 1, &RecvBytes, &Flags,
            &(SI->Overlapped), WorkerRoutine) == SOCKET_ERROR)
        {
            if (WSAGetLastError() != WSA_IO_PENDING )
            {
                printf("WSARecv() failed with error %d\n", WSAGetLastError());
                return;
            }
        }
    }
}
