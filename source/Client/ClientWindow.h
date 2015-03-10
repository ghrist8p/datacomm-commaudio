#ifndef _CLIENT_WINDOW_H_
#define _CLIENT_WINDOW_H_

#include "../GuiLibrary/GuiWindow.h"

class GuiPanel;
class GuiLabel;
class GuiButton;
class GuiStatusBar;
class PlaybackTrackerPanel;

class ClientWindow : public GuiWindow
{
public:
	ClientWindow(HINSTANCE hInst);
	virtual ~ClientWindow();
	virtual void onCreate();

private:
	GuiPanel *topPanel;
	GuiPanel *topPanelStretch;
	GuiPanel *fileContainerPanel;
	GuiPanel *seekPanel;
	GuiLabel *micTargetLabel;
	GuiButton *micTargetButton;
	GuiStatusBar *statusBar;
	PlaybackTrackerPanel *trackerPanel;
};

#endif