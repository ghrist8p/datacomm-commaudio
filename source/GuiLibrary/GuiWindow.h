#ifndef _GUI_WINDOW_H_
#define _GUI_WINDOW_H_

#include "GuiComponent.h"

class GuiWindow : public GuiComponent
{
public:
	GuiWindow(HINSTANCE hInstance);
	GuiWindow(HINSTANCE hInstance, GuiComponent *parent);

	void setExitOnClose(bool exitOnClose);
	void setTitle(LPCWSTR title);
	virtual HWND create(HINSTANCE hInstance, HWND parent);
	virtual void onExit();

private:
	static const LPCWSTR DEFAULT_WIN_CLASS;

	bool exitOnClose;
	bool registerWindowsClass(HINSTANCE hInstance);
};

#endif