#ifndef _SERVER_WINDOW_H_
#define _SERVER_WINDOW_H_

#include "../GuiLibrary/GuiWindow.h"
#include "Server.h"

class GuiListBox;
class GuiPanel;
class GuiLabel;
class GuiButton;
class GuiTextBox;

class ServerWindow : public GuiWindow
{
public:
	ServerWindow(HINSTANCE hInst);
	virtual ~ServerWindow();
	virtual void onCreate();

private:
	GuiListBox *connectedClients;
	GuiPanel *bottomPanel;
	GuiPanel *leftPaddingPanel;
	GuiPanel *inputPanel;
	GuiLabel *tcpPortLabel;
	GuiLabel *udpPortLabel;
	GuiTextBox *tcpPortInput;
	GuiTextBox *udpPortInput;
	GuiButton *connectionButton;
	HFONT labelFont;
	HBRUSH bottomPanelBrush;
	HPEN pen;

    Server * server;

	void createLabelFont();

	static bool toggleConnection(GuiComponent *pThis, UINT command, UINT id, WPARAM wParam, LPARAM lParam, INT_PTR *retval);
    static void newConnHandler( Server * server, void * data );
};

#endif