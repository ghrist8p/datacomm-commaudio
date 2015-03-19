#include "GuiIPv4.h"
#include <CommCtrl.h>

const int GuiIPv4::BUFFER_LENGTH = 16;

GuiIPv4::GuiIPv4(HINSTANCE hInstance, GuiComponent *parent)
	: GuiComponent(hInstance, parent)
{
	buffer = new WCHAR[BUFFER_LENGTH];
}

GuiIPv4::~GuiIPv4()
{
	delete buffer;
}

LPWSTR GuiIPv4::getText()
{
	GetWindowText(hwnd, buffer, BUFFER_LENGTH);

	return buffer;
}

HWND GuiIPv4::create(HINSTANCE hInstance, HWND parent)
{
	return CreateWindowEx(0,
					    WC_IPADDRESS,
						NULL,
						WS_CHILD | WS_VISIBLE,
						0, 0, 0, 0,
						parent,
						NULL,
						hInstance,
						NULL);
}
