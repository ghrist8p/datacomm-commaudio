#ifndef _SERVER_WINDOW_H_
#define _SERVER_WINDOW_H_

#include "../GuiLibrary/GuiWindow.h"

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
	GuiLabel *portLabel;
	GuiTextBox *portInput;
	GuiButton *connectionButton;
	HFONT labelFont;
	HBRUSH bottomPanelBrush;
	HPEN pen;

	void createLabelFont();

	static bool toggleConnection(GuiComponent *pThis, UINT command, UINT id, WPARAM wParam, LPARAM lParam, INT_PTR *retval);
};

#endif