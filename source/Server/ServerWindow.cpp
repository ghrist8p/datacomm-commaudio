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

void ServerWindow::newConnHandler( TCPConnection * server, void * data )
{
	ServerWindow *serverWindow = (ServerWindow*) data;
	serverWindow->connectedClients->addItem(L"New Connection!", -1);
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
