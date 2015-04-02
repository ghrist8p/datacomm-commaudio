#ifndef _CONNECTION_WINDOW_H_
#define _CONNECTION_WINDOW_H_

#include "../GuiLibrary/GuiWindow.h"

class GuiButton;
class GuiLabel;
class GuiTextBox;
class GuiLinearLayout;
class ClientWindow;

class ConnectionWindow : public GuiWindow
{
	public:
		ConnectionWindow(HINSTANCE hInst);
		virtual ~ConnectionWindow();
		virtual void onCreate();

	private:
		static bool connect(GuiComponent *pThis, UINT command, UINT id, WPARAM wParam, LPARAM lParam, INT_PTR *retval);

		ClientWindow *clientWindow;
		GuiLinearLayout *layout;
		GuiLabel *hostLabel;
		GuiLabel *portLabel;
		GuiTextBox *hostInput;
		GuiTextBox *portInput;
		GuiButton *connectButton;
};

#endif