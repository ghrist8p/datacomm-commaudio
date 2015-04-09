#include "ButtonPanel.h"

#define TRANSPARENT_COLOR RGB(0,255,0)

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

ButtonPanel::~ButtonPanel()
{

}

void ButtonPanel::setClickListener(ButtonPanelListener listener)
{
	this->clickListener = listener;
}

bool ButtonPanel::overrideEraseBckgnd(GuiComponent *pThis, UINT command, UINT id, WPARAM wParam, LPARAM lParam, INT_PTR *retval)
{
	*retval = 1;
	return true;
}

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

bool ButtonPanel::onClick(GuiComponent *pThis, UINT command, UINT id, WPARAM wParam, LPARAM lParam, INT_PTR *retval)
{
	ButtonPanel *buttonPanel = (ButtonPanel*)pThis;
	
	buttonPanel->mouseDown = true;
	buttonPanel->buttonState = buttonPanel->down;
	InvalidateRect(buttonPanel->getHWND(), NULL, FALSE);

	return false;
}

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