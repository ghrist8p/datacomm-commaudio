#include "FileListItem.h"
#include "ClientControlThread.h"
#include "../GuiLibrary/GuiScrollList.h"

FileListItem::FileListItem(GuiScrollList *list, ClientWindow *clientWindow, HINSTANCE hInst, SongName song)
	: GuiScrollListItem(list)
{
	this->clientWindow = clientWindow;
	this->setHeight(64);
	this->song = song;
	this->downloading = true;
	background = (HBRUSH)CreateSolidBrush(RGB(20, 20, 20));

	save = LoadBitmap(hInst, L"IMG_SAVE");
	cancelSave = LoadBitmap(hInst, L"IMG_CANCEL_SAVE");
	stream = LoadBitmap(hInst, L"IMG_STREAM");
	this->hoverCursor = LoadCursor(NULL, IDC_HAND);
	this->arrowCursor = LoadCursor(NULL, IDC_ARROW);
	width = 0;
	height = 0;
}

FileListItem::~FileListItem()
{
	DeleteObject(background);
}

void FileListItem::setColours(HBRUSH bg, HPEN brdr, COLORREF fnt)
{
	background = bg;
	border = brdr;
	font = fnt;
}

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
		oldBitmap = SelectObject(buffer, save);
	else
		oldBitmap = SelectObject(buffer, cancelSave);

	GetObject(stream, sizeof(bitmap), &bitmap);
	BitBlt(hdc, drawingArea->right - 56, drawingArea->top + 14, bitmap.bmWidth, bitmap.bmHeight, buffer, 0, 0, SRCCOPY);

	SelectObject(buffer, oldBitmap);

	DeleteDC(buffer);
}

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

void FileListItem::markAsDownloading()
{
	downloading = true;
	InvalidateRect(list->getHWND(), NULL, true);
}

void FileListItem::markAsDownloadingStopped()
{
	downloading = false;
	InvalidateRect(list->getHWND(), NULL, true);
}

bool FileListItem::pointInSaveButton(int x, int y)
{
	if (x > width - 56 && x < width - 56 + 36 && y > 14 && y < 50)
	{
		return true;
	}

	return false;
}

bool FileListItem::pointInPlayButton(int x, int y)
{
	if (x > width - 100 && x < width - 100 + 36 && y > 14 && y < 50)
	{
		return true;
	}

	return false;
}
