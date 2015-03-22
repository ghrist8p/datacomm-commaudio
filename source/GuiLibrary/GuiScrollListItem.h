#ifndef _GUI_SCROLL_LIST_ITEM_H_
#define _GUI_SCROLL_LIST_ITEM_H_

#include <Windows.h>

class GuiScrollList;

class GuiScrollListItem
{
public:
	GuiScrollListItem();
	virtual ~GuiScrollListItem();

	virtual void paint(HDC hdc, LPRECT drawingArea) = 0;
	virtual void onClick(int x, int y){};
	virtual void onMouseMove(int x, int y){};

	void move(int newY);
	void setHeight(int newHeight);

	int getY();
	int getHeight();

private:
	int yPos;
	int height;
};

#endif