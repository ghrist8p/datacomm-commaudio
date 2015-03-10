#include "GuiButton.h"

GuiButton::GuiButton(HINSTANCE hInstance, GuiComponent *parent, UINT id)
	: GuiComponent(hInstance, parent)
{
	setup(L"", L"",
		  WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON);

	this->id = id;
}

void GuiButton::setText(LPCWSTR text)
{
	SetWindowText(hwnd, text);
}

HWND GuiButton::create(HINSTANCE hInstance, HWND parent)
{
	return CreateWindow(
		L"BUTTON",
		L"BTN",
		this->windowStyles,
		0, 0, 0, 0,
		parent,
		(HMENU)id, 
		hInstance,
		NULL);
}