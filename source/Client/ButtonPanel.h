/*-----------------------------------------------------------------------------
-- SOURCE FILE: ButtonPanel.h - This file provides a specialized GUI element
-- that draws images as button states and triggers callbacks when clicked.
--
-- PUBLIC FUNCTIONS:
-- ButtonPanel(HINSTANCE hInstance, GuiComponent *parent, HBITMAP up, HBITMAP down);
-- virtual ~ButtonPanel();
-- void setClickListener(ButtonPanelListener listener);
--
-- REVISIONS:
--
-- DESIGNER: Calvin Rempel
--
-- PROGRAMMER: Calvin Rempel
--
-- NOTES:
-- This class implements a GuiPanel from the GuiLibrary to render itself
-- to screen.
-----------------------------------------------------------------------------*/

#ifndef _BUTTON_PANEL_H_
#define _BUTTON_PANEL_H_

#include "../GuiLibrary/GuiPanel.h"

typedef void (*ButtonPanelListener)(void*);

class ButtonPanel : public GuiPanel
{
public:
	ButtonPanel(HINSTANCE hInstance, GuiComponent *parent, HBITMAP up, HBITMAP down);
	virtual ~ButtonPanel();
	void setClickListener(ButtonPanelListener listener);

private:
	static bool overrideEraseBckgnd(GuiComponent *pThis, UINT command, UINT id, WPARAM wParam, LPARAM lParam, INT_PTR *retval);
	static bool paint(GuiComponent *pThis, UINT command, UINT id, WPARAM wParam, LPARAM lParam, INT_PTR *retval);
	static bool onClick(GuiComponent *pThis, UINT command, UINT id, WPARAM wParam, LPARAM lParam, INT_PTR *retval);
	static bool onClickUp(GuiComponent *pThis, UINT command, UINT id, WPARAM wParam, LPARAM lParam, INT_PTR *retval);
	static bool onMouseMove(GuiComponent *pThis, UINT command, UINT id, WPARAM wParam, LPARAM lParam, INT_PTR *retval);
	static bool onMouseLeave(GuiComponent *pThis, UINT command, UINT id, WPARAM wParam, LPARAM lParam, INT_PTR *retval);

	HCURSOR hoverCursor;
	HCURSOR arrowCursor;
	HBITMAP up;
	HBITMAP down;
	HBITMAP buttonState;
	bool mouseDown;
	ButtonPanelListener clickListener;
};

#endif