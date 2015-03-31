#include "FileListItem.h"
#include "ClientControlThread.h"

FileListItem::FileListItem(ClientWindow *clientWindow, HINSTANCE hInst, LPWSTR filename)
{
	this->clientWindow = clientWindow;
	this->setHeight(64);
	this->filename = filename;
	background = (HBRUSH)CreateSolidBrush(RGB(20, 20, 20));

	save = LoadBitmap(hInst, L"IMG_SAVE");
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
	DrawText(hdc, this->filename, -1, &textArea, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

	// Draw Stream Button
	buffer = CreateCompatibleDC(hdc);
	oldBitmap = SelectObject(buffer, stream);

	GetObject(stream, sizeof(bitmap), &bitmap);
	BitBlt(hdc, drawingArea->right - 100, drawingArea->top + 14, bitmap.bmWidth, bitmap.bmHeight, buffer, 0, 0, SRCCOPY);

	SelectObject(buffer, oldBitmap);

	// Draw Save Button
	oldBitmap = SelectObject(buffer, save);

	GetObject(stream, sizeof(bitmap), &bitmap);
	BitBlt(hdc, drawingArea->right - 56, drawingArea->top + 14, bitmap.bmWidth, bitmap.bmHeight, buffer, 0, 0, SRCCOPY);

	SelectObject(buffer, oldBitmap);

	DeleteDC(buffer);
}

void FileListItem::onClick(int x, int y)
{
	// allocate c style filename string converted from wide char string
	char* cFilename = (char*) malloc(wcslen(filename)+1);
	sprintf(cFilename,"%S",filename);

	// handle the click event
	if (pointInPlayButton(x, y))
	{
		// Request Song Playback
		ClientControlThread::getInstance()->requestChangeStream(cFilename);
	}
	else if (pointInSaveButton(x, y))
	{
		// request song download
		ClientControlThread::getInstance()->requestDownload(cFilename);
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
