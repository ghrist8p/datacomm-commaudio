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

    WIN32_FIND_DATA ffd;
    HANDLE hFind;
    wchar_t* sDir;
    LARGE_INTEGER filesize;

    Server * server;
	bool connected;

	void createLabelFont();

    
static void CALLBACK CompletionROUTINE( IN DWORD dwError
                               , IN DWORD cbTransferred
                               , IN LPWSAOVERLAPPED lpOverlapped
                               , IN DWORD dwFlags );
	static bool toggleConnection(GuiComponent *pThis, UINT command, UINT id, WPARAM wParam, LPARAM lParam, INT_PTR *retval);
    static void newConnHandler( TCPConnection * server, void * data );
};

#endif