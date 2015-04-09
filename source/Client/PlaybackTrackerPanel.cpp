#include "PlaybackTrackerPanel.h"

const int PlaybackTrackerPanel::LINE_WIDTH = 3;
const int PlaybackTrackerPanel::TRACKER_RADIUS = 6;
const int PlaybackTrackerPanel::TRACKER_OUTLINE = 3;

PlaybackTrackerPanel::PlaybackTrackerPanel(HINSTANCE hInstance, GuiComponent *parent)
	: GuiPanel(hInstance, parent)
{
	buffered = 0;
	played = 0;
	inactivePen = CreatePen(PS_SOLID, PlaybackTrackerPanel::LINE_WIDTH, RGB(60, 60, 60));
	bufferedPen = CreatePen(PS_SOLID, 0, RGB(0, 162, 232));
	bufferedBrush = CreateSolidBrush(RGB(0, 162, 232));
	playedPen = CreatePen(PS_SOLID, PlaybackTrackerPanel::LINE_WIDTH, RGB(128, 0, 128));
	trackerActiveBrush = CreateSolidBrush(RGB(128, 0, 128));
	trackerInactiveBrush = CreateSolidBrush(RGB(255, 255, 255));;
	trackerActivePen = CreatePen(PS_SOLID, PlaybackTrackerPanel::TRACKER_OUTLINE, RGB(255, 255, 255));
	trackerInactivePen = CreatePen(PS_SOLID, PlaybackTrackerPanel::TRACKER_OUTLINE, RGB(128, 0, 128));
	hoverCursor = LoadCursor(NULL, IDC_HAND);
	arrowCursor = LoadCursor(NULL, IDC_ARROW);

	trackerBrush = trackerInactiveBrush;
	trackerPen = trackerInactivePen;

	mouseDown = false;
	this->backgroundBrush = (HBRUSH) GetStockObject(BLACK_BRUSH);
	this->addMessageListener(WM_ERASEBKGND, PlaybackTrackerPanel::overrideEraseBckgnd, this);
	this->addMessageListener(WM_PAINT, PlaybackTrackerPanel::paint, this);
	this->addMessageListener(WM_LBUTTONDOWN, PlaybackTrackerPanel::onClick, this);
	this->addMessageListener(WM_LBUTTONUP, PlaybackTrackerPanel::onClickUp, this);
	this->addMessageListener(WM_MOUSEMOVE, PlaybackTrackerPanel::onMouseMove, this);
	this->addMessageListener(WM_MOUSELEAVE, PlaybackTrackerPanel::onMouseLeave, this);

	DWORD id;
	CreateThread(NULL, NULL, PlaybackTrackerPanel::drawAgain, &this->hwnd, NULL, &id);
}


PlaybackTrackerPanel::~PlaybackTrackerPanel()
{
	DeleteObject(inactivePen);
	DeleteObject(bufferedPen);
	DeleteObject(playedPen);
	DeleteObject(trackerBrush);
	DeleteObject(trackerActiveBrush);
	DeleteObject(trackerInactiveBrush);
	DeleteObject(trackerActivePen);
	DeleteObject(trackerInactivePen);
}

void PlaybackTrackerPanel::setPercentageBuffered(double percent)
{
	// Constrain buffer bar to full width
	if (percent < 0)
		buffered = 0;
	else if (percent > 1)
		percent = 1;
	else
		buffered = percent;

	// If buffer is now smaller than tracker, update the tracker
	setTrackerPercentage(played, true);
}

void PlaybackTrackerPanel::setTrackerPercentage(double percent, bool hasPriority)
{
	// Disallow wresting control from the moust action
	if (mouseDown && !hasPriority)
		return;

	// Constrain tracker to buffer width
	if (percent < 0)
		played = 0;
	else if (percent > buffered)
		played = buffered;
	else
		played = percent;

	RECT rect;
	GetClientRect(getHWND(), &rect);
	RedrawWindow(getHWND(), &rect, NULL, RDW_ERASENOW);
}

bool PlaybackTrackerPanel::onClick(GuiComponent *pThis, UINT command, UINT id, WPARAM wParam, LPARAM lParam, INT_PTR *retval)
{
	PlaybackTrackerPanel *tracker = (PlaybackTrackerPanel*) pThis;
	int xPos = LOWORD(lParam);
	double percent = ((double) xPos) / tracker->getWidth();

	tracker->trackerBrush = tracker->trackerActiveBrush;
	tracker->trackerPen = tracker->trackerActivePen;
	tracker->setTrackerPercentage(percent, true);

	RECT rect;
	GetClientRect(tracker->getHWND(), &rect);
	RedrawWindow(tracker->getHWND(), &rect, NULL, RDW_ERASENOW);

	tracker->mouseDown = true;

	return true;
}

bool PlaybackTrackerPanel::onClickUp(GuiComponent *pThis, UINT command, UINT id, WPARAM wParam, LPARAM lParam, INT_PTR *retval)
{
	PlaybackTrackerPanel *tracker = (PlaybackTrackerPanel*)pThis;
	tracker->mouseDown = false;
	tracker->trackerBrush = tracker->trackerInactiveBrush;
	tracker->trackerPen = tracker->trackerInactivePen;

	RECT rect;
	GetClientRect(tracker->getHWND(), &rect);
	RedrawWindow(tracker->getHWND(), &rect, NULL, RDW_ERASENOW);
	SendMessage(tracker->getHWND(), WM_SEEK, (WPARAM)(tracker->played * 1000), 0);
	return true;
}

bool PlaybackTrackerPanel::onMouseMove(GuiComponent *pThis, UINT command, UINT id, WPARAM wParam, LPARAM lParam, INT_PTR *retval)
{
	PlaybackTrackerPanel *tracker = (PlaybackTrackerPanel*)pThis;

	int xPos = LOWORD(lParam);

	SetCursor(tracker->hoverCursor);

	// If mouse button is held, allow the user to slide the tracker
	if (tracker->mouseDown)
	{
		double percent = ((double)xPos) / tracker->getWidth();
		tracker->setTrackerPercentage(percent, true);
		RECT rect;
		GetClientRect(tracker->getHWND(), &rect);
		RedrawWindow(tracker->getHWND(), &rect, NULL, RDW_ERASENOW);
	}

	return false;
}

bool PlaybackTrackerPanel::onMouseLeave(GuiComponent *pThis, UINT command, UINT id, WPARAM wParam, LPARAM lParam, INT_PTR *retval)
{
	PlaybackTrackerPanel *tracker = (PlaybackTrackerPanel*)pThis;
	SetCursor(tracker->arrowCursor);

	if (!tracker->mouseDown)
	{
		tracker->trackerBrush = tracker->trackerInactiveBrush;
		tracker->trackerPen = tracker->trackerInactivePen;
		RECT rect;
		GetClientRect(tracker->getHWND(), &rect);
		RedrawWindow(tracker->getHWND(), &rect, NULL, RDW_ERASENOW);
	}

	return false;
}

bool PlaybackTrackerPanel::overrideEraseBckgnd(GuiComponent *pThis, UINT command, UINT id, WPARAM wParam, LPARAM lParam, INT_PTR *retval)
{
	*retval = 1;
	return true;
}

bool PlaybackTrackerPanel::paint(GuiComponent *pThis, UINT command, UINT id, WPARAM wParam, LPARAM lParam, INT_PTR *retval)
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(pThis->getHWND(), &ps);

	PlaybackTrackerPanel *tracker = (PlaybackTrackerPanel*) pThis;
	int totalWidth = pThis->getWidth();
	int y = pThis->getHeight() / 2.0;

	// Draw Background
	SelectObject(hdc, tracker->backgroundBrush);
	SelectObject(hdc, tracker->borderPen);
	Rectangle(hdc, 0, 0, totalWidth, y * 2);

	// Draw Background Line
	SelectObject(hdc, tracker->inactivePen);
	MoveToEx(hdc, 0, y, 0);
	LineTo(hdc, totalWidth, y);

	// Draw Buffer Line
	int halfLineWidth = PlaybackTrackerPanel::LINE_WIDTH / 2.0;
	SelectObject(hdc, tracker->bufferedPen);
	SelectObject(hdc, tracker->bufferedBrush);
	Rectangle(hdc, 0, y - halfLineWidth, totalWidth * tracker->buffered, y + halfLineWidth);

	// Draw Tracked Line
	SelectObject(hdc, tracker->playedPen);
	MoveToEx(hdc, 0, y, 0);
	LineTo(hdc, totalWidth * tracker->played, y);

	// Draw Tracker
	int tx = totalWidth * tracker->played;
	int radius = PlaybackTrackerPanel::TRACKER_RADIUS;

	SelectObject(hdc, tracker->trackerPen);
	SelectObject(hdc, tracker->trackerBrush);
	Ellipse(hdc, tx - radius, y - radius, tx + radius, y + radius);


	EndPaint(pThis->getHWND(), &ps);

	return true;
}

DWORD WINAPI PlaybackTrackerPanel::drawAgain(LPVOID hwnd)
{
	while (true)
	{
		Sleep(100);
		InvalidateRect(*((HWND*)hwnd), NULL, false);
	}

	return 0;
}