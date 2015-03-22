#ifndef _PLAYBACK_TRACKER_H_
#define _PLAYBACK_TRACKER_H_

#include "../GuiLibrary/GuiPanel.h"

class PlaybackTrackerPanel : public GuiPanel
{
public:
	PlaybackTrackerPanel(HINSTANCE hInstance, GuiComponent *parent);
	virtual ~PlaybackTrackerPanel();

	void setPercentageBuffered(double percent);
	void setTrackerPercentage(double percent, bool hasPriority = false);

private:
	static const int LINE_WIDTH;
	static const int TRACKER_RADIUS;
	static const int TRACKER_OUTLINE;
	static bool overrideEraseBckgnd(GuiComponent *pThis, UINT command, UINT id, WPARAM wParam, LPARAM lParam, INT_PTR *retval);
	static bool paint(GuiComponent *pThis, UINT command, UINT id, WPARAM wParam, LPARAM lParam, INT_PTR *retval);
	static bool onClick(GuiComponent *pThis, UINT command, UINT id, WPARAM wParam, LPARAM lParam, INT_PTR *retval);
	static bool onClickUp(GuiComponent *pThis, UINT command, UINT id, WPARAM wParam, LPARAM lParam, INT_PTR *retval);
	static bool onMouseMove(GuiComponent *pThis, UINT command, UINT id, WPARAM wParam, LPARAM lParam, INT_PTR *retval);
	static bool onMouseLeave(GuiComponent *pThis, UINT command, UINT id, WPARAM wParam, LPARAM lParam, INT_PTR *retval);

	double buffered;
	double played;
	HPEN inactivePen;
	HPEN bufferedPen;
	HBRUSH bufferedBrush;
	HPEN playedPen;
	HBRUSH trackerActiveBrush;
	HBRUSH trackerInactiveBrush;
	HPEN trackerActivePen;
	HPEN trackerInactivePen;
	HCURSOR hoverCursor;
	HCURSOR arrowCursor;
	HBRUSH trackerBrush;
	HPEN trackerPen;
	bool mouseDown;
};

#endif