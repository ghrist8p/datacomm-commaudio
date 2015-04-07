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
class UDPSocket;
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

	GuiPanel *tcpInputPanel;
	GuiPanel *udpInputPanel;
	GuiPanel *playlistInputPanel;

	GuiLabel *tcpPortLabel;
	GuiLabel *udpPortLabel;
	GuiLabel *playlistLabel;

	GuiTextBox *tcpPortInput;
	GuiTextBox *udpPortInput;
	GuiTextBox *playlistInput;

	GuiButton *connectionButton;

    HFONT labelFont;
	HBRUSH bottomPanelBrush;
	HPEN pen;

	void createLabelFont();

    Server * server;
	bool connected;

    UDPSocket* udpSock;

    uint8_t * receiveMessage(  TCPConnection * from );

    static void CALLBACK receiveCallback( ULONG_PTR param );
    static void CALLBACK receive( DWORD dwError
                                , DWORD cbTransferred
                                , LPWSAOVERLAPPED lpOverlapped
                                , DWORD dwFlags );
    static DWORD WINAPI MicThread(LPVOID lpParameter);
    DWORD ThreadStart(void);
	static bool toggleConnection(GuiComponent *pThis, UINT command, UINT id, WPARAM wParam, LPARAM lParam, INT_PTR *retval);
    static void newConnHandler( TCPConnection * server, void * data );
};

#endif
