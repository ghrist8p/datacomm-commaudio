/*-----------------------------------------------------------------------------
-- SOURCE FILE: FileListItem.h - This file provides a renderable elements to
-- be drawn in a GuiScrollList view for a file list.
--
-- PUBLIC FUNCTIONS:
-- FileListItem(GuiScrollList *list, ClientWindow *clientWindow, HINSTANCE hInst, SongName filename);
-- virtual ~FileListItem();
-- void setColours(HBRUSH bg, HPEN brdr, COLORREF fnt);
-- virtual void paint(HDC hdc, LPRECT drawingArea);
-- virtual void onClick(int x, int y);
-- virtual void onMouseMove(int x, int y);
-- void markAsDownloading();
-- void markAsDownloadingStopped();
--
-- REVISIONS:
--
-- DESIGNER: Calvin Rempel
--
-- PROGRAMMER: Calvin Rempel
--
-- NOTES:
-- This file provides a renderable elements to
-- be drawn in a GuiScrollList view for a file list.
-----------------------------------------------------------------------------*/

#ifndef _FILE_LIST_ITEM_H_
#define _FILE_LIST_ITEM_H_

#include "../GuiLibrary/GuiScrollListItem.h"

class GuiScrollList;
class ClientWindow;

class FileListItem : public GuiScrollListItem
{
public:
	FileListItem(GuiScrollList *list, ClientWindow *clientWindow, HINSTANCE hInst, SongName filename);
	virtual ~FileListItem();
	
	void setColours(HBRUSH bg, HPEN brdr, COLORREF fnt);
	virtual void paint(HDC hdc, LPRECT drawingArea);

	virtual void onClick(int x, int y);
	virtual void onMouseMove(int x, int y);

	void markAsDownloading();
	void markAsDownloadingStopped();
private:
	bool pointInPlayButton(int x, int y);
	bool pointInSaveButton(int x, int y);

	ClientWindow *clientWindow;
	bool downloading;
	int width, height;
	HBRUSH background;
	HPEN border;
	HBITMAP save;
	HBITMAP cancelSave;
	HBITMAP stream;
	COLORREF font;
	SongName song;
	HCURSOR hoverCursor;
	HCURSOR arrowCursor;
};

#endif
