/*-----------------------------------------------------------------------------
-- SOURCE FILE: ButtonPanel.cpp - This file provides a specialized GUI element
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

#include "ButtonPanel.h"

#define TRANSPARENT_COLOR RGB(0,255,0)

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: ButtonPanel
--
-- REVISIONS:
--
-- DESIGNER: Calvin Rempel
--
-- PROGRAMMER: Calvin Rempel
--
-- INTERFACE: ButtonPanel(HINSTANCE hInstance, GuiComponent *parent, HBITMAP up, HBITMAP down)
--		HINSTANCE hInstance : the instance of the application
--      GuiComponent parent : the parent component of the button
--      HBITMAP up          : the image to display when button is up
--      HBITMAP down        : the image to display when button is down    
--
-- NOTES: Creates a new ButtonPanel
-------------------------------------------------------------------------------------------------*/
ButtonPanel::ButtonPanel(HINSTANCE hInstance, GuiComponent *parent, HBITMAP up, HBITMAP down)
	: GuiPanel(hInstance, parent)
{
	this->up = up;
	this->down = down;
	this->buttonState = up;
	this->mouseDown = false;

	this->hoverCursor = LoadCursor(NULL, IDC_HAND);
	this->arrowCursor = LoadCursor(NULL, IDC_ARROW);

	this->backgroundBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);
	this->addMessageListener(WM_ERASEBKGND, ButtonPanel::overrideEraseBckgnd, this);
	this->addMessageListener(WM_PAINT, ButtonPanel::paint, this);
	this->addMessageListener(WM_LBUTTONDOWN, ButtonPanel::onClick, this);
	this->addMessageListener(WM_LBUTTONUP, ButtonPanel::onClickUp, this);
	this->addMessageListener(WM_MOUSEMOVE, ButtonPanel::onMouseMove, this);
	this->addMessageListener(WM_MOUSELEAVE, ButtonPanel::onMouseLeave, this);
}

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: ~ButtonPanel
--
-- REVISIONS:
--
-- DESIGNER: Calvin Rempel
--
-- PROGRAMMER: Calvin Rempel
--
-- INTERFACE: ~ButtonPanel() 
--
-- NOTES: Destroy the ButtonPanel
-------------------------------------------------------------------------------------------------*/
ButtonPanel::~ButtonPanel()
{
}

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: setClickListener
--
-- REVISIONS:
--
-- DESIGNER: Calvin Rempel
--
-- PROGRAMMER: Calvin Rempel
--
-- INTERFACE: setClickListener(ButtonPanelListener listener)
--      ButtonPanelListener listener : a callback that is called when the button is clicked.
--
-- NOTES: This function allows for a single callback to be assigned to the "onClick" role of the
-- button.
-------------------------------------------------------------------------------------------------*/
void ButtonPanel::setClickListener(ButtonPanelListener listener)
{
	this->clickListener = listener;
}

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: overrideEraseBckgnd
--
-- REVISIONS:
--
-- DESIGNER: Calvin Rempel
--
-- PROGRAMMER: Calvin Rempel
--
-- INTERFACE: overrideEraseBckgnd(...)
--
-- NOTES: A function applied to WM_ERASEBCKGND to prevent the image from being erased.
-------------------------------------------------------------------------------------------------*/
bool ButtonPanel::overrideEraseBckgnd(GuiComponent *pThis, UINT command, UINT id, WPARAM wParam, LPARAM lParam, INT_PTR *retval)
{
	*retval = 1;
	return true;
}

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: paint
--
-- REVISIONS:
--
-- DESIGNER: Calvin Rempel
--
-- PROGRAMMER: Calvin Rempel
--
-- INTERFACE: paint(...)
--
-- NOTES: A function applied to WM_PAINT to draw the custom control.
-------------------------------------------------------------------------------------------------*/
bool ButtonPanel::paint(GuiComponent *pThis, UINT command, UINT id, WPARAM wParam, LPARAM lParam, INT_PTR *retval)
{
	ButtonPanel *button = (ButtonPanel*) pThis;

	PAINTSTRUCT ps;
	HDC hdc;
	HDC buffer;
	BITMAP bitmap;
	HBITMAP activeState = button->buttonState;
	HGDIOBJ oldBitmap;

	hdc = BeginPaint(pThis->getHWND(), &ps);
	HDC backbuffDC = CreateCompatibleDC(hdc);

	HBITMAP backbuffer = CreateCompatibleBitmap( hdc, button->getWidth(), button->getHeight());
    int savedDC = SaveDC(backbuffDC);
    SelectObject( backbuffDC, backbuffer );

	// Draw Button Image
	buffer = CreateCompatibleDC(backbuffDC);
	oldBitmap = SelectObject(buffer, (HGDIOBJ) activeState);
	DWORD error = GetLastError();

	GetObject(activeState, sizeof(bitmap), &bitmap);
	TransparentBlt(backbuffDC, 0, 0, bitmap.bmWidth, bitmap.bmHeight,
				   buffer, 0, 0, bitmap.bmWidth, bitmap.bmHeight,
				   TRANSPARENT_COLOR);

	SelectObject(buffer, oldBitmap);

	BitBlt(hdc,0,0,button->getWidth(),button->getHeight(),backbuffDC,0,0,SRCCOPY);
    RestoreDC(backbuffDC,savedDC);

	DeleteDC(buffer);
	DeleteObject(backbuffer);
    DeleteDC(backbuffDC);

	EndPaint(pThis->getHWND(), &ps);

	return true;
}

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: onClick
--
-- REVISIONS:
--
-- DESIGNER: Calvin Rempel
--
-- PROGRAMMER: Calvin Rempel
--
-- INTERFACE: onClick(...)
--
-- NOTES: When the button is clicked, the image is changed.
-------------------------------------------------------------------------------------------------*/
bool ButtonPanel::onClick(GuiComponent *pThis, UINT command, UINT id, WPARAM wParam, LPARAM lParam, INT_PTR *retval)
{
	ButtonPanel *buttonPanel = (ButtonPanel*)pThis;
	
	buttonPanel->mouseDown = true;
	buttonPanel->buttonState = buttonPanel->down;
	InvalidateRect(buttonPanel->getHWND(), NULL, FALSE);

	return false;
}

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: onClickUp
--
-- REVISIONS:
--
-- DESIGNER: Calvin Rempel
--
-- PROGRAMMER: Calvin Rempel
--
-- INTERFACE: onClickUp(...)
--
-- NOTES: When the button is un-clicked, the image is changed and the click callback is triggered.
-------------------------------------------------------------------------------------------------*/
bool ButtonPanel::onClickUp(GuiComponent *pThis, UINT command, UINT id, WPARAM wParam, LPARAM lParam, INT_PTR *retval)
{
	ButtonPanel *buttonPanel = (ButtonPanel*)pThis;

	if (buttonPanel->mouseDown)
	{
		buttonPanel->mouseDown = false;
		buttonPanel->buttonState = buttonPanel->up;
		InvalidateRect(buttonPanel->getHWND(), NULL, FALSE);

		// Call Click Listener
		buttonPanel->clickListener(pThis);
	}

	return false;
}

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: onMouseMove
--
-- REVISIONS:
--
-- DESIGNER: Calvin Rempel
--
-- PROGRAMMER: Calvin Rempel
--
-- INTERFACE: onMouseMove(...)
--
-- NOTES:
-------------------------------------------------------------------------------------------------*/
bool ButtonPanel::onMouseMove(GuiComponent *pThis, UINT command, UINT id, WPARAM wParam, LPARAM lParam, INT_PTR *retval)
{
	ButtonPanel *button = (ButtonPanel*) pThis;

	SetCursor(button->hoverCursor);

	// If mouse button is held, allow the user to slide the tracker
	if (button->mouseDown)
	{
		button->buttonState = button->down;
		InvalidateRect(button->getHWND(), NULL, FALSE);
	}

	return false;
}

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: onMouseLeave
--
-- REVISIONS:
--
-- DESIGNER: Calvin Rempel
--
-- PROGRAMMER: Calvin Rempel
--
-- INTERFACE: onMouseLeave(...)
--
-- NOTES: When the mouse is moved, and the mouse button is held down, un-press the button if the
-- mouse leaves the client area.
-------------------------------------------------------------------------------------------------*/
bool ButtonPanel::onMouseLeave(GuiComponent *pThis, UINT command, UINT id, WPARAM wParam, LPARAM lParam, INT_PTR *retval)
{
	ButtonPanel *button = (ButtonPanel*) pThis;
	SetCursor(button->arrowCursor);

	if (!button->mouseDown)
	{
		button->buttonState = button->up;
		InvalidateRect(button->getHWND(), NULL, FALSE);
	}

	return false;
}