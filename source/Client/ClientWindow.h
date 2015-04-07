#ifndef _CLIENT_WINDOW_H_
#define _CLIENT_WINDOW_H_

#include "../GuiLibrary/GuiWindow.h"
#include "Sockets.h"


class ConnectionWindow;
class GuiPanel;
class GuiTextBox;
class GuiLabel;
class GuiButton;
class GuiStatusBar;
class PlaybackTrackerPanel;
class ButtonPanel;
class GuiScrollList;
class MessageQueue;
class MicReader;
class MusicBuffer;
class ClientControlThread;

class ClientWindow : public GuiWindow
{
	friend class ClientControlThread;
public:
	ClientWindow(HINSTANCE hInst);
	virtual ~ClientWindow();
	virtual void onCreate();

	void addRemoteFile(SongName);

private:
	static void onClickPlay(void*);
	static void onClickStop(void*);
	static bool onClickMic(GuiComponent *_pThis, UINT command, UINT id, WPARAM wParam, LPARAM lParam, INT_PTR *retval);
	static bool onMicStop(GuiComponent *_pThis, UINT command, UINT id, WPARAM wParam, LPARAM lParam, INT_PTR *retval);
	static bool onSeek(GuiComponent *_pThis, UINT command, UINT id, WPARAM wParam, LPARAM lParam, INT_PTR *retval);
	static DWORD WINAPI MicThread(LPVOID lpParameter);
	DWORD ThreadStart(void);

	bool recording;
	bool requestingRecorderStop;

    char voiceTargetAddress[STR_LEN];

	DataPacket voicePacket;
	MessageQueue *micMQueue;
	MicReader *micReader;
    MusicBuffer* musicfile;

	HBITMAP playButtonUp;
	HBITMAP playButtonDown;
	HBITMAP stopButtonUp;
	HBITMAP stopButtonDown;
	HBRUSH darkBackground;
	HBRUSH lightBackground;
	HBRUSH accentBrush;
	HPEN borderPen;
	HPEN nullPen;
	COLORREF textColor;

	GuiPanel *topPanel;
	GuiPanel *topPanelStretch;
	GuiLabel *voiceTargetLabel;
	GuiTextBox *voiceTargetInput;
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
	ButtonPanel *stopButton;

	UDPSocket* udpSock;
};

#endif
