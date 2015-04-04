#ifndef _SERVER_WINDOW_H_
#define _SERVER_WINDOW_H_

#include "../GuiLibrary/GuiWindow.h"

#define BUFSIZE 64

class Server;
class GuiListBox;
class GuiPanel;
class GuiLabel;
class GuiButton;
class GuiTextBox;
class TCPSocket;
struct TCPConnection;

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
    
	void createLabelFont();

    Server * server;
	bool connected;

    uint8_t * receiveMessage(  TCPConnection * from );

    static void CALLBACK receiveCallback( ULONG_PTR param );
    static void CALLBACK receive( DWORD dwError
                                , DWORD cbTransferred
                                , LPWSAOVERLAPPED lpOverlapped
                                , DWORD dwFlags );

	static bool toggleConnection(GuiComponent *pThis, UINT command, UINT id, WPARAM wParam, LPARAM lParam, INT_PTR *retval);
    static void newConnHandler( TCPConnection * server, void * data );
};

#endif