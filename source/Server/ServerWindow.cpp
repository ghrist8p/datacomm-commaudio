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
}


ServerWindow::~ServerWindow()
{
	DeleteObject(labelFont);
	DeleteObject(bottomPanelBrush);
	DeleteObject(pen);
	
	delete connectedClients;
	delete bottomPanel;
	delete leftPaddingPanel;
	delete tcpInputPanel;
	delete udpInputPanel;
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
	setSize(700, 325);

	// Create Window Components
	connectedClients = new GuiListBox(hInst, this);
	bottomPanel = new GuiPanel(hInst, this);
	leftPaddingPanel = new GuiPanel(hInst, bottomPanel);
	tcpInputPanel = new GuiPanel(hInst, bottomPanel);
	udpInputPanel = new GuiPanel(hInst, bottomPanel);
	tcpPortLabel = new GuiLabel(hInst, tcpInputPanel);
	udpPortLabel = new GuiLabel(hInst, udpInputPanel);
	tcpPortInput = new GuiTextBox(hInst, tcpInputPanel, false);
	udpPortInput = new GuiTextBox(hInst, udpInputPanel, false);
	connectionButton = new GuiButton(hInst, tcpInputPanel, IDB_CONNECTION_TOGGLE);

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
	bottomPanel->setPreferredSize(0, 72);
	layout->addComponent(bottomPanel);

	// Get the Bottom Panel Layout
	layout = (GuiLinearLayout*)bottomPanel->getLayoutManager();
	layout->setHorizontal(false);

	// Add the Left Padding Panel to the Bottom Panel Layout
	leftPaddingPanel->init();
	//layout->addComponent(leftPaddingPanel, &layoutProps);

	// Add the TCP Input Panel to the Bottom Panel Layout
	tcpInputPanel->init();
	tcpInputPanel->setPreferredSize(1000, 0);
	tcpInputPanel->addCommandListener(BN_CLICKED, toggleConnection, this);
	layoutProps.bottomMargin = 5;
	layoutProps.topMargin = 5;
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
	layoutProps.topMargin = 5;
	layout->addComponent(tcpPortLabel, &layoutProps);
	tcpPortInput->init();
	tcpPortInput->setPreferredSize(256, 0);
	layout->addComponent(tcpPortInput, &layoutProps);

	connectionButton->init();
	connectionButton->setText(L"Connect");
	layoutProps.weight = 1;
	layout->addComponent(connectionButton, &layoutProps);
    
	layout = (GuiLinearLayout*)bottomPanel->getLayoutManager();
    // Add the TCP Input Panel to the Bottom Panel Layout
	udpInputPanel->init();
	udpInputPanel->setPreferredSize(1000, 0);
	udpInputPanel->addCommandListener(BN_CLICKED, toggleConnection, this);
	layoutProps.bottomMargin = 5;
	layoutProps.topMargin = 5;
	layoutProps.leftMargin = 5;
	layoutProps.rightMargin = 5;
	layout->addComponent(udpInputPanel, &layoutProps);

	// Get the Input Panel Layout
	layout = (GuiLinearLayout*)udpInputPanel->getLayoutManager();
	layout->setHorizontal(true);

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

void ServerWindow::newConnHandler( Server * server, void * data )
{
	ServerWindow *serverWindow = (ServerWindow*) data;
	serverWindow->connectedClients->addItem(L"New Connection!", -1);
}

bool ServerWindow::toggleConnection(GuiComponent *pThis, UINT command, UINT id, WPARAM wParam, LPARAM lParam, INT_PTR *retval)
{
    
	ServerWindow *serverWindow = (ServerWindow*) pThis;
	/**
	 * START SERVER LISTENING HERE
	 */
    unsigned short tcpPort = _wtoi( serverWindow->tcpPortInput->getText() );
    unsigned short udpPort = _wtoi( serverWindow->udpPortInput->getText() );
    unsigned short groupAddress = inet_addr( MULTICAST_ADDRESS );

    serverWindow->server = new Server( tcpPort, newConnHandler, serverWindow, groupAddress, udpPort );
    serverWindow->server->startTCP();
    serverWindow->server->startUDP();

	serverWindow->connectedClients->addItem(L"Button Pressed!", -1);

	return true;
}