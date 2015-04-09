#include "ConnectionWindow.h"

#include "ClientWindow.h"
#include "../GuiLibrary/GuiLinearLayout.h"
#include "../GuiLibrary/GuiButton.h"
#include "../GuiLibrary/GuiTextBox.h"
#include "../GuiLibrary/GuiLabel.h"

ConnectionWindow::ConnectionWindow(HINSTANCE hInst)
	: GuiWindow(hInst) 
{
	setup(windowClass, NULL, windowStyles);
	setExitOnClose(true);

	clientWindow = new ClientWindow(hInst);
	clientWindow->init();

	addCommandListener(0, ConnectionWindow::connect, this);
}

ConnectionWindow::~ConnectionWindow()
{
	delete hostLabel;
	delete portLabel;
	delete hostInput;
	delete portInput;
	delete connectButton;
	delete clientWindow;
}

void ConnectionWindow::onCreate()
{
	setTitle(L"Connection Settings");
	setMiniumSize(325, 160);
	setMaximumSize(325, 160);
	setSize(325, 160);

	GuiLinearLayout::Properties layoutProps;
	layout = (GuiLinearLayout*) getLayoutManager();
	layout->zeroProperties(&layoutProps);
	layout->setHorizontal(false);
	layoutProps.leftMargin = 6;
	layoutProps.rightMargin = 6;
	layoutProps.topMargin = 0;

	// Host Label
	hostLabel = new GuiLabel(hInst, this);
	hostLabel->init();
	hostLabel->setText(L"Host Address: ");

	// Port Label
	portLabel = new GuiLabel(hInst, this);
	portLabel->init();
	portLabel->setText(L"Port: ");

	// Host Input
	hostInput = new GuiTextBox(hInst, this, false);
	hostInput->setPreferredSize(0, 28);
	hostInput->init();
	
	// Port Input
	portInput = new GuiTextBox(hInst, this, false);
	portInput->setPreferredSize(0, 28);
	portInput->init();

	// Create Connection Button
	connectButton = new GuiButton(hInst, this, 0);
	connectButton->init();
	connectButton->setPreferredSize(0, 32);
	connectButton->setText(L"Connect");

	// Add buttons to layout
	layout->addComponent(hostLabel, &layoutProps);
	layout->addComponent(hostInput, &layoutProps);
	layout->addComponent(portLabel, &layoutProps);
	layout->addComponent(portInput, &layoutProps);
	layout->addComponent(connectButton, &layoutProps);
}

bool ConnectionWindow::connect(GuiComponent *pThis, UINT command, UINT id, WPARAM wParam, LPARAM lParam, INT_PTR *retval)
{
	ConnectionWindow *window = (ConnectionWindow*) pThis;

    size_t retvall;

    LPWSTR wHost = window->hostInput->getText();
    char * host = (char *) malloc( wcslen( wHost ) + 1 );

    wcstombs_s( &retvall               // size_t *pReturnValue,
              , host                   // char *mbstr,
              , wcslen( wHost ) + 1    // size_t sizeInBytes,
              , wHost                  // const wchar_t *wcstr,
              , wcslen( wHost ) + 1 ); // size_t count 

    ClientControlThread * cct = ClientControlThread::getInstance();
    cct->connect( host, _wtoi( window->portInput->getText() ) );

	window->clientWindow->startConnection();
	window->clientWindow->setVisible(true);
	window->setVisible(false);

	return true;
}