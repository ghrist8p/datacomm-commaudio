/*-----------------------------------------------------------------------------
-- SOURCE FILE: ServerWindow.h - This file provides a controller for the
-- server aspect of the ComAudio project.
--
-- PUBLIC FUNCTIONS:
-- ServerWindow(HINSTANCE hInst);
-- virtual ~ServerWindow();
-- virtual void onCreate();
--
-- REVISIONS:
--
-- DESIGNER: Calvin Rempel
--
-- PROGRAMMER: Calvin Rempel
--
-- NOTES:
-- This class implements a GuiWindow that is used as the controller for the
-- server application.
-----------------------------------------------------------------------------*/

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

/*-----------------------------------------------------------------------------
-- CLASS: ServerWindow
--
-- DESCRIPTION: This class implements a GuiWindow that is used as the controller
-- for the server application.
-----------------------------------------------------------------------------*/
class ServerWindow : public GuiWindow
{
	friend class ServerControlThread;
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
	static bool toggleConnection(GuiComponent *pThis, UINT command, UINT id, WPARAM wParam, LPARAM lParam, INT_PTR *retval);
    static void newConnHandler( TCPConnection * server, void * data );
};

#endif
