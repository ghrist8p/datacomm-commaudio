#ifndef _FILE_LIST_ITEM_H_
#define _FILE_LIST_ITEM_H_

#include "../GuiLibrary/GuiScrollListItem.h"

class ClientWindow;

class FileListItem : public GuiScrollListItem
{
public:
	FileListItem(ClientWindow *clientWindow, HINSTANCE hInst, LPWSTR filename);
	virtual ~FileListItem();
	
	void setColours(HBRUSH bg, HPEN brdr, COLORREF fnt);
	virtual void paint(HDC hdc, LPRECT drawingArea);

	virtual void onClick(int x, int y);
	virtual void onMouseMove(int x, int y);


private:
	bool pointInPlayButton(int x, int y);
	bool pointInSaveButton(int x, int y);

	ClientWindow *clientWindow;
	int width, height;
	HBRUSH background;
	HPEN border;
	HBITMAP save;
	HBITMAP stream;
	COLORREF font;
	LPWSTR filename;
	HCURSOR hoverCursor;
	HCURSOR arrowCursor;
};

#endif
