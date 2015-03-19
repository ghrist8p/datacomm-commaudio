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
	delete inputPanel;
	delete portLabel;
	delete portInput;
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
	inputPanel = new GuiPanel(hInst, bottomPanel);
	portLabel = new GuiLabel(hInst, inputPanel);
	portInput = new GuiTextBox(hInst, inputPanel, false);
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
	bottomPanel->setPreferredSize(0, 36);
	layout->addComponent(bottomPanel);

	// Get the Bottom Panel Layout
	layout = (GuiLinearLayout*)bottomPanel->getLayoutManager();
	layout->setHorizontal(true);

	// Add the Left Padding Panel to the Bottom Panel Layout
	leftPaddingPanel->init();
	layout->addComponent(leftPaddingPanel, &layoutProps);

	// Add the Input Panel to the Bottom Panel Layout
	inputPanel->init();
	inputPanel->setPreferredSize(400, 0);
	inputPanel->addCommandListener(BN_CLICKED, toggleConnection, this);
	layoutProps.bottomMargin = 5;
	layoutProps.topMargin = 5;
	layoutProps.leftMargin = 5;
	layoutProps.rightMargin = 5;
	layout->addComponent(inputPanel, &layoutProps);

	// Get the Input Panel Layout
	layout = (GuiLinearLayout*)inputPanel->getLayoutManager();
	layout->setHorizontal(true);

	// Add Inputs to Input Panel
	portLabel->init();
	createLabelFont();
	portLabel->setText(L"Port:");
	layoutProps.leftMargin = 0;
	layoutProps.rightMargin = 0;
	layoutProps.topMargin = 5;
	layout->addComponent(portLabel, &layoutProps);

	layout->zeroProperties(&layoutProps);
	layoutProps.leftMargin = 5;

	portInput->init();
	portInput->setPreferredSize(256, 0);
	layout->addComponent(portInput, &layoutProps);

	connectionButton->init();
	connectionButton->setText(L"Connect");
	layoutProps.weight = 1;
	layout->addComponent(connectionButton, &layoutProps);
}

void ServerWindow::createLabelFont()
{
	LOGFONT logFont;
	memset(&logFont, 0, sizeof(logFont));
	logFont.lfHeight = -15; // see PS
	labelFont = CreateFontIndirect(&logFont);

	SendMessage(portLabel->getHWND(), WM_SETFONT, (WPARAM)labelFont, TRUE);
}

bool ServerWindow::toggleConnection(GuiComponent *pThis, UINT command, UINT id, WPARAM wParam, LPARAM lParam, INT_PTR *retval)
{
	/**
	 * START SERVER LISTENING HERE
	 */

	ServerWindow *serverWindow = (ServerWindow*) pThis;
	serverWindow->connectedClients->addItem(L"Button Pressed!", -1);

	return true;
}