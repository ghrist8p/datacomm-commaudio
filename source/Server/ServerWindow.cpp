#include "ServerWindow.h"

#include "../GuiLibrary/GuiTextBox.h"
#include "../GuiLibrary/GuiPanel.h"
#include "../GuiLibrary/GuiLabel.h"
#include "../GuiLibrary/GuiListBox.h"
#include "../GuiLibrary/GuiButton.h"
#include "../GuiLibrary/GuiLinearLayout.h"
#include "resource.h"

ServerWindow::ServerWindow(HINSTANCE hInst)
	: GuiWindow(hInst)
{
	setup(windowClass, NULL, windowStyles);
	setMiniumSize(700, 325);
	setExitOnClose(true);

	bottomPanelBrush = CreateSolidBrush(RGB(255, 0, 0));
	pen = CreatePen(0, 2, RGB(0, 0, 255));
	connected = false;
	hFind = NULL;

	sDir = L"C:\\Users\\Eric\\Documents\\Visual Studio 2012\\Projects\\commaudio\\Debug\\music\\*.wav";
	hFind = FindFirstFile(sDir, &ffd);
	if (INVALID_HANDLE_VALUE == hFind)
	{
        int err = GetLastError();
		MessageBeep(1);
		return;
	}
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
	delete tcpPortLabel;
    delete udpPortLabel;
	delete tcpPortInput;
	delete udpPortInput;
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
	tcpPortLabel = new GuiLabel(hInst, inputPanel);
	udpPortLabel = new GuiLabel(hInst, inputPanel);
	tcpPortInput = new GuiTextBox(hInst, inputPanel, false);
	udpPortInput = new GuiTextBox(hInst, inputPanel, false);
	connectionButton = new GuiButton(hInst, inputPanel, IDB_CONNECTION_TOGGLE);

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
	bottomPanel->setPreferredSize(0, 38);
	layout->addComponent(bottomPanel);

	// Get the Bottom Panel Layout
	layout = (GuiLinearLayout*)bottomPanel->getLayoutManager();
	layout->setHorizontal(false);

	// Add the Left Padding Panel to the Bottom Panel Layout
	leftPaddingPanel->init();
	//layout->addComponent(leftPaddingPanel, &layoutProps);

	// Add the TCP Input Panel to the Bottom Panel Layout
	inputPanel->init();
	inputPanel->setPreferredSize(700, 0);
	inputPanel->addCommandListener(BN_CLICKED, toggleConnection, this);
	layoutProps.bottomMargin = 5;
	layoutProps.topMargin = 5;
	layoutProps.leftMargin = 5;
	layoutProps.rightMargin = 5;
	layout->addComponent(inputPanel, &layoutProps);

	// Get the Input Panel Layout
	layout = (GuiLinearLayout*)inputPanel->getLayoutManager();
	layout->setHorizontal(true);

	// Add Inputs to TCP Input Panel
	layout->zeroProperties(&layoutProps);
	layoutProps.leftMargin = 5;

    tcpPortLabel->init();
	createLabelFont();
	tcpPortLabel->setText(L"TCP:");
	layoutProps.leftMargin = 0;
	layoutProps.rightMargin = 0;
	layoutProps.topMargin = 5;
	layout->addComponent(tcpPortLabel, &layoutProps);
	tcpPortInput->init();
	tcpPortInput->setPreferredSize(256, 0);
	layout->addComponent(tcpPortInput, &layoutProps);

	udpPortLabel->init();
	createLabelFont();
	udpPortLabel->setText(L"UDP:");
	layoutProps.leftMargin = 0;
	layoutProps.rightMargin = 0;
	layoutProps.topMargin = 5;
	layout->addComponent(udpPortLabel, &layoutProps);

	udpPortInput->init();
	udpPortInput->setPreferredSize(256, 0);
	layout->addComponent(udpPortInput, &layoutProps);

	connectionButton->init();
	connectionButton->setText(L"Start");
	layoutProps.weight = 1;
	layout->addComponent(connectionButton, &layoutProps);

    // reading all files in the music folder
	do
	{
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			//_tprintf(TEXT("  %s   <DIR>\n"), ffd.cFileName);
		}
		else
		{
			filesize.LowPart = ffd.nFileSizeLow;
			filesize.HighPart = ffd.nFileSizeHigh;
            WCHAR info[256];
            wsprintf(info,L"%s size: %d",ffd.cFileName,filesize.QuadPart);
			this->connectedClients->addItem(info, -1);
		}
	} while (FindNextFile(hFind, &ffd) != 0);

	FindClose(hFind);
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

    // send playlist
    while( true )
    {
        // WSASend( blah->connection->sock   // _In_   SOCKET s
        //            ,    // _In_   LPWSABUF lpBuffers
        //            ,    // _In_   DWORD dwBufferCount
        //            ,    // _Out_  LPDWORD lpNumberOfBytesSent
        //            ,    // _In_   DWORD dwFlags
        //            ,    // _In_   LPWSAOVERLAPPED lpOverlapped
        //            , ); // _In_   LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine

    }
}

bool ServerWindow::toggleConnection(GuiComponent *pThis, UINT command, UINT id, WPARAM wParam, LPARAM lParam, INT_PTR *retval)
{

	ServerWindow *serverWindow = (ServerWindow*) pThis;

	/**
	 * STOP SERVER LISTENING HERE
	 */
	if (serverWindow->connected)
	{
		serverWindow->server->disconnect();
		serverWindow->tcpPortInput->setEnabled(true);
		serverWindow->udpPortInput->setEnabled(true);
		serverWindow->connectionButton->setText(L"Start Server");
		serverWindow->connected = false;
	}
	/**
	* START SERVER LISTENING HERE
	*/
	else
	{
		unsigned short tcpPort = _wtoi(serverWindow->tcpPortInput->getText());
		unsigned short udpPort = _wtoi(serverWindow->udpPortInput->getText());
		unsigned short groupAddress = inet_addr(MULTICAST_ADDRESS);

		serverWindow->server = new Server(tcpPort, newConnHandler, serverWindow, groupAddress, udpPort);
		if (serverWindow->server->startTCP())
		{
			if (serverWindow->server->startUDP())
			{
				serverWindow->tcpPortInput->setEnabled(false);
				serverWindow->udpPortInput->setEnabled(false);
				serverWindow->connectionButton->setText(L"Close Server");
				serverWindow->connected = true;
			}
		}
	}

	//serverWindow->connectedClients->addItem(L"Button Pressed!", -1);

	return true;
}
