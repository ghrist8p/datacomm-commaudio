#include "ServerWindow.h"

#include "../GuiLibrary/GuiTextBox.h"
#include "../GuiLibrary/GuiPanel.h"
#include "../GuiLibrary/GuiLabel.h"
#include "../GuiLibrary/GuiListBox.h"
#include "../GuiLibrary/GuiButton.h"
#include "../GuiLibrary/GuiLinearLayout.h"
#include "resource.h"
#include "../Common.h"
#include "Server.h"
#include "../Buffer/MessageQueue.h"
#include "ServerControlThread.h"
#include "../Client/Sockets.h"

/**
 * element that is put into the message queue.
 */
union MsgqElement
{
    int index;
    char string[STR_LEN];
};

#define MCAPA	30
#define MSGQ_CAPACITY 30
#define MSGQ_ELEM_SIZE sizeof(MsgqElement)

ServerWindow::ServerWindow(HINSTANCE hInst)
	: GuiWindow(hInst)
{
	setup(windowClass, NULL, windowStyles);
	setMiniumSize(700, 325);
	setExitOnClose(true);

	bottomPanelBrush = CreateSolidBrush(RGB(255, 0, 0));
	pen = CreatePen(0, 2, RGB(0, 0, 255));
	connected = false;

    DWORD useless;
    CreateThread(NULL, 0, MicThread, (void*)this, 0, &useless);
}

DWORD WINAPI ServerWindow::MicThread(LPVOID lpParameter)
{
    ServerWindow* This = (ServerWindow*)lpParameter;
    return This->ThreadStart();
}

DWORD ServerWindow::ThreadStart(void)
{
    int useless;
    int length;

    DataPacket voicePacket;

    udpSock = new UDPSocket(7392,new MessageQueue(10,10));
    udpSock->setGroup(MULTICAST_ADDR,0);

    voicePacket.index = 0;

    // continuously send voice data over the network when it becomes available
    char sound[DATA_LEN];
    FILE* fp = fopen("C:\\Users\\Eric\\Downloads\\Egoist_-_Extra_Terrestrial_Biological_Entities.wav","rb");
    while(fread(sound,1,DATA_LEN,fp))
    {
        ++(voicePacket.index);
        memcpy(voicePacket.data, sound, DATA_LEN);
        udpSock->sendtoGroup(MUSICSTREAM,&voicePacket,sizeof(voicePacket));
    }

    return 0;
}

ServerWindow::~ServerWindow()
{
	DeleteObject(labelFont);
	DeleteObject(bottomPanelBrush);
	DeleteObject(pen);

	delete connectedClients;
	delete bottomPanel;
	delete leftPaddingPanel;

    delete inputPanel;

	delete tcpInputPanel;
	delete udpInputPanel;
	delete playlistInputPanel;

	delete tcpPortLabel;
    delete udpPortLabel;
    delete playlistLabel;

	delete tcpPortInput;
	delete udpPortInput;
	delete playlistInput;
	delete connectionButton;
}

void ServerWindow::onCreate()
{
	// Set Window Properties
	setTitle(L"CommAudio Server");
	setSize(750, 325);

	// Create Window Components
	connectedClients = new GuiListBox(hInst, this);
	bottomPanel = new GuiPanel(hInst, this);
	leftPaddingPanel = new GuiPanel(hInst, bottomPanel);

    inputPanel = new GuiPanel(hInst, bottomPanel);

	tcpInputPanel = new GuiPanel(hInst, inputPanel);
	udpInputPanel = new GuiPanel(hInst, inputPanel);
	playlistInputPanel = new GuiPanel(hInst, inputPanel);

	tcpPortLabel = new GuiLabel(hInst, tcpInputPanel);
	udpPortLabel = new GuiLabel(hInst, udpInputPanel);
	playlistLabel = new GuiLabel(hInst, playlistInputPanel);

	tcpPortInput = new GuiTextBox(hInst, tcpInputPanel, false);
	udpPortInput = new GuiTextBox(hInst, udpInputPanel, false);
	playlistInput = new GuiTextBox(hInst, playlistInputPanel, false);

	connectionButton = new GuiButton(hInst, bottomPanel, IDB_CONNECTION_TOGGLE);

	// Get the windows default vertical linear layout
	GuiLinearLayout *layout = (GuiLinearLayout*)getLayoutManager();
	GuiLinearLayout::Properties layoutProps;
	layout->zeroProperties(&layoutProps);
	layout->setHorizontal(false);

	// Add Client Listbox to the Window Layout
	layoutProps.weight = 1;
	connectedClients->init();
	connectedClients->setPreferredSize(0, 0);
	layout->addComponent(connectedClients, &layoutProps);

	// Add Bottom Panel to the Window Layout
	bottomPanel->init();
	bottomPanel->setPreferredSize(0, 100);
	bottomPanel->addCommandListener(BN_CLICKED, toggleConnection, this);
	layout->addComponent(bottomPanel);

	// Get the Bottom Panel Layout
	layout = (GuiLinearLayout*)bottomPanel->getLayoutManager();
	layout->setHorizontal(true);

	// Add the Left Padding Panel to the Bottom Panel Layout
	leftPaddingPanel->init();
	//layout->addComponent(leftPaddingPanel, &layoutProps);

	inputPanel->init();
	inputPanel->setPreferredSize(400, 120);
	layoutProps.bottomMargin = 5;
	layoutProps.topMargin = 5;
	layoutProps.leftMargin = 5;
	layoutProps.rightMargin = 5;
	layout->addComponent(inputPanel, &layoutProps);


	layout = (GuiLinearLayout*)inputPanel->getLayoutManager();
	layout->setHorizontal(false);

	// Add the TCP Input Panel to the Bottom Panel Layout
	tcpInputPanel->init();
	tcpInputPanel->setPreferredSize(400, 30);
	layoutProps.bottomMargin = 0;
	layoutProps.topMargin = 0;
	layoutProps.leftMargin = 5;
	layoutProps.rightMargin = 5;
	layout->addComponent(tcpInputPanel, &layoutProps);

	// Get the Input Panel Layout
	layout = (GuiLinearLayout*)tcpInputPanel->getLayoutManager();
	layout->setHorizontal(true);

	// Add Inputs to TCP Input Panel
	layout->zeroProperties(&layoutProps);
	layoutProps.leftMargin = 5;

    tcpPortLabel->init();
	createLabelFont();
	tcpPortLabel->setText(L"TCP:");
	layoutProps.leftMargin = 0;
	layoutProps.rightMargin = 0;
	layoutProps.topMargin = 0;
	layout->addComponent(tcpPortLabel, &layoutProps);
	tcpPortInput->init();
	tcpPortInput->setPreferredSize(256, 0);
	layout->addComponent(tcpPortInput, &layoutProps);


	layout = (GuiLinearLayout*)inputPanel->getLayoutManager();

	// Add the TCP Input Panel to the Bottom Panel Layout
	udpInputPanel->init();
	udpInputPanel->setPreferredSize(400, 30);
	layoutProps.bottomMargin = 0;
	layoutProps.topMargin = 0;
	layoutProps.leftMargin = 5;
	layoutProps.rightMargin = 5;
	layout->addComponent(udpInputPanel, &layoutProps);

	layout = (GuiLinearLayout*)udpInputPanel->getLayoutManager();
	layout->setHorizontal(true);

	udpPortLabel->init();
	createLabelFont();
	udpPortLabel->setText(L"UDP:");
	layoutProps.leftMargin = 0;
	layoutProps.rightMargin = 0;
	layoutProps.topMargin = 0;
	layout->addComponent(udpPortLabel, &layoutProps);

	udpPortInput->init();
	udpPortInput->setPreferredSize(256, 0);
	layout->addComponent(udpPortInput, &layoutProps);


	layout = (GuiLinearLayout*)inputPanel->getLayoutManager();

	// Add the TCP Input Panel to the Bottom Panel Layout
	playlistInputPanel->init();
	playlistInputPanel->setPreferredSize(400, 30);
	layoutProps.bottomMargin = 0;
	layoutProps.topMargin = 0;
	layoutProps.leftMargin = 5;
	layoutProps.rightMargin = 5;
	layout->addComponent(playlistInputPanel, &layoutProps);

	layout = (GuiLinearLayout*)playlistInputPanel->getLayoutManager();
	layout->setHorizontal(true);

	playlistLabel->init();
	createLabelFont();
	playlistLabel->setText(L"Playlist:");
	layoutProps.leftMargin = 0;
	layoutProps.rightMargin = 0;
	layoutProps.topMargin = 0;
	layout->addComponent(playlistLabel, &layoutProps);

	playlistInput->init();
	playlistInput->setPreferredSize(256, 0);
	layout->addComponent(playlistInput, &layoutProps);

    layout = (GuiLinearLayout*)bottomPanel->getLayoutManager();

	connectionButton->init();
	connectionButton->setText(L"Start");
	layoutProps.weight = 1;
	layout->addComponent(connectionButton, &layoutProps);
}

void ServerWindow::createLabelFont()
{
	LOGFONT logFont;
	memset(&logFont, 0, sizeof(logFont));
	logFont.lfHeight = -15; // see PS
	labelFont = CreateFontIndirect(&logFont);

	SendMessage(tcpPortLabel->getHWND(), WM_SETFONT, (WPARAM)labelFont, TRUE);
	SendMessage(udpPortLabel->getHWND(), WM_SETFONT, (WPARAM)labelFont, TRUE);
}

typedef struct
{
    WSAOVERLAPPED   overlapped;
    WSABUF          wsabuf;
    DWORD           bytesToRead;
    DWORD           cursor;
    uint8_t       * buf;
    ServerWindow  * window;
    TCPConnection * connection;
}
RECEIVER;

void CALLBACK ServerWindow::receive( IN DWORD dwError
                                   , IN DWORD cbTransferred
                                   , IN LPWSAOVERLAPPED lpOverlapped
                                   , IN DWORD dwFlags )
{
    RECEIVER * receiver = (RECEIVER *) lpOverlapped;

    memcpy( receiver->buf + receiver->cursor, receiver->wsabuf.buf, cbTransferred );
    receiver->cursor += cbTransferred;
    receiver->bytesToRead -= cbTransferred;

    if( cbTransferred > 0 )
    {
        DWORD flags = 0;

        memset( &receiver->overlapped, 0, sizeof( WSAOVERLAPPED ) );
        memset( receiver->wsabuf.buf, 0, receiver->wsabuf.len );

        WSARecv( receiver->connection->sock // _In_   SOCKET s,
               , &receiver->wsabuf          //_Inout_ LPWSABUF lpBuffers,
               , 1                          //_In_    DWORD dwBufferCount,
               , NULL                       //_Out_   LPDWORD lpNumberOfBytesRecvd,
               , &flags                     //_Inout_ LPDWORD lpFlags,
               , &receiver->overlapped      //_In_    LPWSAOVERLAPPED lpOverlapped,
               , receive );                 //_In_    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    }
    else
    {
        delete receiver->wsabuf.buf;
        WSASetEvent( receiver->connection->signal );
    }
}

uint8_t * ServerWindow::receiveMessage( TCPConnection * from )
{
    // setup receiver structure to receive the header
    RECEIVER * receiver   = new RECEIVER;
    memset( receiver, 0, sizeof( WSAOVERLAPPED ) );
    receiver->wsabuf.len  = BUFSIZE;
    receiver->wsabuf.buf  = new char[ BUFSIZE ];
    memset( receiver->wsabuf.buf, 0, BUFSIZE );
    receiver->bytesToRead = sizeof( MessageHeader );
    receiver->cursor      = 0;
    receiver->buf         = (uint8_t *) malloc( sizeof( MessageHeader ) );
    memset( receiver->buf, 0, sizeof( MessageHeader ) );
    receiver->window      = this;
    receiver->connection  = from;

    // submit to server
    server->submitCompletionRoutine( receiveCallback, receiver );

    WSAEVENT   eventArray[1];
    eventArray[0] = from->signal;

    DWORD retval;

    // wait for compleition
    retval = WSAWaitForMultipleEvents( 1            // _In_ DWORD            cEvents
                                         , eventArray   // _In_ const WSAEVENT * lphEvents
                                         , FALSE        // _In_ BOOL             fWaitAll
                                         , WSA_INFINITE // _In_ DWORD            dwTimeout
                                         , TRUE );      // _In_ BOOL             fAlertable

    // On error
    if( retval == WSA_WAIT_FAILED )
    {
        wchar_t errorStr[256] = {0};
        swprintf( errorStr, 256, L"WSAWaitForMultipleEvents() failed: %d", WSAGetLastError() );
        MessageBox(NULL, errorStr, L"Error", MB_ICONERROR);
    }

    WSAResetEvent( from->signal );

    // setup receiver structure to receive the data
    receiver->buf = (uint8_t *) realloc( receiver->buf, sizeof( MessageHeader ) + ((MessageHeader *) receiver->buf)->size );
    memset( receiver->buf + sizeof( MessageHeader ), 0, ((MessageHeader *) receiver->buf)->size );
    receiver->bytesToRead = ((MessageHeader *) receiver->buf)->size;
    receiver->cursor      = sizeof( MessageHeader );

    // submit to server
    server->submitCompletionRoutine( receiveCallback, receiver );

    // wait for compleition
    retval = WSAWaitForMultipleEvents( 1            // _In_ DWORD            cEvents
                                         , eventArray   // _In_ const WSAEVENT * lphEvents
                                         , FALSE        // _In_ BOOL             fWaitAll
                                         , WSA_INFINITE // _In_ DWORD            dwTimeout
                                         , TRUE );      // _In_ BOOL             fAlertable

    // On error
    if( retval == WSA_WAIT_FAILED )
    {
        wchar_t errorStr[256] = {0};
        swprintf( errorStr, 256, L"WSAWaitForMultipleEvents() failed: %d", WSAGetLastError() );
        MessageBox(NULL, errorStr, L"Error", MB_ICONERROR);
    }

    WSAResetEvent( from->signal );

    // extract data
    uint8_t * toReturn = receiver->buf;

    // cleanup
    delete [] receiver->wsabuf.buf;
    delete receiver;

    //return data
    return toReturn;
}

void ServerWindow::receiveCallback( ULONG_PTR param )
{
    RECEIVER * receiver = (RECEIVER *) param;

    DWORD flags = 0;

    WSARecv( receiver->connection->sock // _In_   SOCKET s,
           , &receiver->wsabuf          //_Inout_ LPWSABUF lpBuffers,
           , 1                          //_In_    DWORD dwBufferCount,
           , NULL                       //_Out_   LPDWORD lpNumberOfBytesRecvd,
           , &flags                     //_Inout_ LPDWORD lpFlags,
           , &receiver->overlapped      //_In_    LPWSAOVERLAPPED lpOverlapped,
           , receive );                 //_In_    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine

}

void ServerWindow::newConnHandler( TCPConnection * connection, void * data )
{
	ServerWindow *serverWindow = (ServerWindow*) data;
	serverWindow->connectedClients->addItem(L"New Connection!", -1);

	MessageQueue* msgQueue = new MessageQueue(MCAPA, DATA_BUFSIZE);
	TCPSocket*  new_client = new TCPSocket(connection->sock, msgQueue);
    ServerControlThread::getInstance()->addConnection( new_client );
}

bool ServerWindow::toggleConnection(GuiComponent *pThis, UINT command, UINT id, WPARAM wParam, LPARAM lParam, INT_PTR *retval)
{
	ServerWindow *serverWindow = (ServerWindow*) pThis;
    ServerControlThread * sct = ServerControlThread::getInstance();

	/**
	 * STOP SERVER LISTENING HERE
	 */
	if (serverWindow->connected)
	{
        serverWindow->connectedClients->addItem(L"Stoping...", -1);

		serverWindow->server->disconnect();
		serverWindow->tcpPortInput->setEnabled(true);
		serverWindow->udpPortInput->setEnabled(true);
		serverWindow->playlistInput->setEnabled(true);
		serverWindow->connectionButton->setText(L"Start Server");
		serverWindow->connected = false;
	}
	/**
	* START SERVER LISTENING HERE
	*/
	else
	{
        serverWindow->connectedClients->addItem(L"Starting...", -1);

        sct->setPlaylist( new Playlist( serverWindow->playlistInput->getText() ) );

        for( std::vector< SongName >::iterator it = sct->getPlaylist()->playlist.begin()
           ; it != sct->getPlaylist()->playlist.end()
           ; ++it )
        {
            serverWindow->connectedClients->addItem( it->filepath, -1 );
        }

		unsigned short tcpPort = _wtoi(serverWindow->tcpPortInput->getText());
		unsigned short udpPort = _wtoi(serverWindow->udpPortInput->getText());
		unsigned short groupAddress = inet_addr(MULTICAST_ADDR);

		serverWindow->server = new Server(tcpPort, newConnHandler, serverWindow, groupAddress, udpPort);
		if (serverWindow->server->startTCP())
		{
            sct->setUDPSocket( new UDPSocket( udpPort, new MessageQueue( MSGQ_CAPACITY, MSGQ_ELEM_SIZE ) ) );
            sct->start();

		    serverWindow->tcpPortInput->setEnabled(false);
		    serverWindow->udpPortInput->setEnabled(false);
		    serverWindow->playlistInput->setEnabled(false);
		    serverWindow->connectionButton->setText(L"Close Server");
		    serverWindow->connected = true;
		}
	}

	//serverWindow->connectedClients->addItem(L"Button Pressed!", -1);

	return true;
}
