/*-----------------------------------------------------------------------------
-- SOURCE FILE: ConnectionWindow.h - This file provides a controller for the
-- connection logic on the client side.
--
-- PUBLIC FUNCTIONS:
-- ConnectionWindow(HINSTANCE hInst);
-- virtual ~ConnectionWindow();
-- virtual void onCreate();
--
-- REVISIONS:
--
-- DESIGNER: Calvin Rempel
--
-- PROGRAMMER: Calvin Rempel
--
-- NOTES:
-----------------------------------------------------------------------------*/

#ifndef _CONNECTION_WINDOW_H_
#define _CONNECTION_WINDOW_H_

#include "../GuiLibrary/GuiWindow.h"
#include "ClientControlThread.h"

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