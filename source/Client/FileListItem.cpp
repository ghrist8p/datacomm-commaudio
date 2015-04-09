/*-----------------------------------------------------------------------------
-- SOURCE FILE: FileListItem.cpp - This file provides a renderable elements to
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

#include "FileListItem.h"
#include "ClientControlThread.h"
#include "../GuiLibrary/GuiScrollList.h"

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: FileListItem
--
-- REVISIONS:
--
-- DESIGNER: Calvin Rempel
--
-- PROGRAMMER: Calvin Rempel
--
-- INTERFACE: FileListItem(GuiScrollList *list, ClientWindow *clientWindow, HINSTANCE hInst, SongName song)
--      GuiScrollList *list        : The List that the item belongs to.
--      ClientWindow *clientWindow : The executing window
--      HINSTANCE hInst            : The instance of the application
--      SongName song              : Song information to be displayed on the list item
--
-- NOTES: Creates a new FileListItem
-------------------------------------------------------------------------------------------------*/
FileListItem::FileListItem(GuiScrollList *list, ClientWindow *clientWindow, HINSTANCE hInst, SongName song)
	: GuiScrollListItem(list)
{
	this->clientWindow = clientWindow;
	this->setHeight(64);
	this->song = song;
	this->downloading = false;
	background = (HBRUSH)CreateSolidBrush(RGB(20, 20, 20));

	save = LoadBitmap(hInst, L"IMG_SAVE");
	cancelSave = LoadBitmap(hInst, L"IMG_CANCEL_SAVE");
	stream = LoadBitmap(hInst, L"IMG_STREAM");
	this->hoverCursor = LoadCursor(NULL, IDC_HAND);
	this->arrowCursor = LoadCursor(NULL, IDC_ARROW);
	width = 0;
	height = 0;
}

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: ~FileListItem
--
-- REVISIONS:
--
-- DESIGNER: Calvin Rempel
--
-- PROGRAMMER: Calvin Rempel
--
-- INTERFACE: ~FileListItem()
--
-- NOTES: Free Resources
-------------------------------------------------------------------------------------------------*/
FileListItem::~FileListItem()
{
	DeleteObject(background);
}

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: setColours
--
-- REVISIONS:
--
-- DESIGNER: Calvin Rempel
--
-- PROGRAMMER: Calvin Rempel
--
-- INTERFACE: setColours(HBRUSH bg, HPEN brdr, COLORREF fnt)
--      HBRUSH bg     : The background colour
--      HPEN brdr     : The colour of the border
--      COLORREF fnt  : The colour of the font.
--
-- NOTES: Changes the colours of the item
-------------------------------------------------------------------------------------------------*/
void FileListItem::setColours(HBRUSH bg, HPEN brdr, COLORREF fnt)
{
	background = bg;
	border = brdr;
	font = fnt;
}

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: paint
--
-- REVISIONS:
--
-- DESIGNER: Calvin Rempel
--
-- PROGRAMMER: Calvin Rempel
--
-- INTERFACE: paint(HDC hdc, LPRECT drawingArea)
--      HDC hdc            : The device context to draw to
--      LPRECT drawingArea : The are to draw into.
--
-- NOTES: Renders the custom list item.
-------------------------------------------------------------------------------------------------*/
void FileListItem::paint(HDC hdc, LPRECT drawingArea)
{
	HDC buffer;
	HGDIOBJ oldBitmap;
	BITMAP bitmap;
	RECT textArea;

	textArea.left = drawingArea->left + 4;
	textArea.top = drawingArea->top + 4;
	textArea.bottom = drawingArea->bottom - 4;
	textArea.right = drawingArea->right - 168;

	width = drawingArea->right - drawingArea->left;
	height = drawingArea->bottom - drawingArea->top;

	SelectObject(hdc, background);
	SelectObject(hdc, border);
	Rectangle(hdc, drawingArea->left, drawingArea->top, drawingArea->right, drawingArea->bottom);

	// Draw Filename
	SetTextColor(hdc, RGB(0, 168, 232));
	SetBkMode(hdc, TRANSPARENT);
	DrawText(hdc, this->song.filepath, -1, &textArea, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

	// Draw Stream Button
	buffer = CreateCompatibleDC(hdc);
	oldBitmap = SelectObject(buffer, stream);

	GetObject(stream, sizeof(bitmap), &bitmap);
	BitBlt(hdc, drawingArea->right - 100, drawingArea->top + 14, bitmap.bmWidth, bitmap.bmHeight, buffer, 0, 0, SRCCOPY);

	SelectObject(buffer, oldBitmap);

	// Draw Save Button
	if (downloading)
		oldBitmap = SelectObject(buffer, cancelSave);
	else
		oldBitmap = SelectObject(buffer, save);

	GetObject(stream, sizeof(bitmap), &bitmap);
	BitBlt(hdc, drawingArea->right - 56, drawingArea->top + 14, bitmap.bmWidth, bitmap.bmHeight, buffer, 0, 0, SRCCOPY);

	SelectObject(buffer, oldBitmap);

	DeleteDC(buffer);
}

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: onClick
--
-- REVISIONS:
--
-- DESIGNER: Calvin Rempel
--
-- PROGRAMMER: Calvin Rempel
--
-- INTERFACE: onClick(int x, int y)
--
-- NOTES:
-------------------------------------------------------------------------------------------------*/
void FileListItem::onClick(int x, int y)
{
	// allocate c style filename string converted from wide char string
    size_t retval;
    char* cFilename = (char*) malloc(wcslen(song.filepath)+1);
    wcstombs_s( &retval                    // size_t *pReturnValue,
              , cFilename                  // char *mbstr,
              , wcslen(song.filepath)+1    // size_t sizeInBytes,
              , song.filepath              // const wchar_t *wcstr,
              , wcslen(song.filepath)+1 ); // size_t count

	// handle the click event
	if (pointInPlayButton(x, y))
	{
		// Request Song Playback
        ClientControlThread::getInstance()->requestChangeStream(song.id);
	}
	else if (pointInSaveButton(x, y))
	{
		// request song download
		if (downloading)
		{
			ClientControlThread::getInstance()->cancelDownload(song.id);
			markAsDownloadingStopped();
		}
		else
		{
			ClientControlThread::getInstance()->requestDownload(song.id);
			markAsDownloading();
		}
	}

	// free c style filename string
	free(cFilename);
}

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: onMouseMove
--
-- REVISIONS:
--
-- DESIGNER: Calvin Rempel
--
-- PROGRAMMER: Calvin Rempel
--
-- INTERFACE: onMouseMove(int x, int y)
--
-- NOTES:
-------------------------------------------------------------------------------------------------*/
void FileListItem::onMouseMove(int x, int y)
{
	if (pointInPlayButton(x, y) || pointInSaveButton(x, y))
	{
		SetCursor(hoverCursor);
	}
	else
	{
		SetCursor(arrowCursor);
	}
}

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: markAsDownloading
--
-- REVISIONS:
--
-- DESIGNER: Calvin Rempel
--
-- PROGRAMMER: Calvin Rempel
--
-- INTERFACE: markAsDownloading()
--
-- NOTES:
-------------------------------------------------------------------------------------------------*/
void FileListItem::markAsDownloading()
{
	downloading = true;
	InvalidateRect(list->getHWND(), NULL, false);
}

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: markAsDownloadingStopped
--
-- REVISIONS:
--
-- DESIGNER: Calvin Rempel
--
-- PROGRAMMER: Calvin Rempel
--
-- INTERFACE: markAsDownloadingStopped()
--
-- NOTES:
-------------------------------------------------------------------------------------------------*/
void FileListItem::markAsDownloadingStopped()
{
	downloading = false;
	InvalidateRect(list->getHWND(), NULL, false);
}

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: pointInSaveButton
--
-- REVISIONS:
--
-- DESIGNER: Calvin Rempel
--
-- PROGRAMMER: Calvin Rempel
--
-- INTERFACE: pointInSaveButton(int x, int y)
--
-- NOTES:
-------------------------------------------------------------------------------------------------*/
bool FileListItem::pointInSaveButton(int x, int y)
{
	if (x > width - 56 && x < width - 56 + 36 && y > 14 && y < 50)
	{
		return true;
	}

	return false;
}

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: pointInPlayButton
--
-- REVISIONS:
--
-- DESIGNER: Calvin Rempel
--
-- PROGRAMMER: Calvin Rempel
--
-- INTERFACE: pointInPlayButton(int x, int y)
--
-- NOTES:
-------------------------------------------------------------------------------------------------*/
bool FileListItem::pointInPlayButton(int x, int y)
{
	if (x > width - 100 && x < width - 100 + 36 && y > 14 && y < 50)
	{
		return true;
	}

	return false;
}
