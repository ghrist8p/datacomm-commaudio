#ifndef _CLIENT_WINDOW_H_
#define _CLIENT_WINDOW_H_

#include "../GuiLibrary/GuiWindow.h"
#include "Sockets.h"

class GuiPanel;
class GuiLabel;
class GuiButton;
class GuiStatusBar;
class PlaybackTrackerPanel;
class ButtonPanel;
class GuiScrollList;

class ClientWindow : public GuiWindow
{
public:
	ClientWindow(HINSTANCE hInst);
	virtual ~ClientWindow();
	virtual void onCreate();

	void addRemoteFile(LPWSTR filename);

private:
	static void onClickPlay(void*);

	HBITMAP playButtonUp;
	HBITMAP playButtonDown;
	HBRUSH darkBackground;
	HBRUSH lightBackground;
	HBRUSH accentBrush;
	HPEN borderPen;
	HPEN nullPen;
	COLORREF textColor;

	GuiPanel *topPanel;
	GuiPanel *topPanelStretch;
	GuiScrollList *fileContainerPanel;
	GuiPanel *seekPanel;
	GuiLabel *micTargetLabel;
	GuiButton *micTargetButton;
	GuiStatusBar *statusBar;
	PlaybackTrackerPanel *trackerPanel;
	GuiPanel *buttonSpacer1;
	GuiPanel *buttonSpacer2;
	GuiPanel *bottomSpacer;
	ButtonPanel *playButton;

    TCPSocket * tcpSocket;
};

#endif