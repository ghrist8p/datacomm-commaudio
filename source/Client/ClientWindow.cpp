#include "ClientWindow.h"

#include "resource.h"

#include "../GuiLibrary/GuiPanel.h"
#include "../GuiLibrary/GuiStatusBar.h"
#include "../GuiLibrary/GuiLinearLayout.h"
#include "../GuiLibrary/GuiButton.h"
#include "../GuiLibrary/GuiLabel.h"
#include "PlaybackTrackerPanel.h"

ClientWindow::ClientWindow(HINSTANCE hInst)
	: GuiWindow(hInst)
{
	setup(windowClass, NULL, windowStyles);
	setMiniumSize(700, 325);
	setExitOnClose(true);
}


ClientWindow::~ClientWindow()
{
}

void ClientWindow::onCreate()
{
	setTitle(L"CommAudio Client");
	setSize(700, 325);

	// Create Elements
	topPanel = new GuiPanel(hInst, this);
	topPanelStretch = new GuiPanel(hInst, topPanel);
	fileContainerPanel = new GuiPanel(hInst, this);
	seekPanel = new GuiPanel(hInst, this);
	micTargetLabel = new GuiLabel(hInst, topPanel);
	micTargetButton = new GuiButton(hInst, topPanel, IDB_MIC_TOGGLE);
	statusBar = new GuiStatusBar(hInst, this);
	trackerPanel = new PlaybackTrackerPanel(hInst, this);

	// Create Layouts
	GuiLinearLayout *layout = (GuiLinearLayout*) this->getLayoutManager();
	GuiLinearLayout::Properties layoutProps;
	layout->zeroProperties(&layoutProps);
	layout->setHorizontal(false);

	// Add Top Panel to Layout
	topPanel->init();
	topPanel->setPreferredSize(0, 64);
	layout->addComponent(topPanel);

	// Add File Panel to layout
	fileContainerPanel->init();

	layoutProps.weight = 1;
	fileContainerPanel->setPreferredSize(0, 0);
	layout->addComponent(fileContainerPanel, &layoutProps);

	// Add Tracker Panel to layout
	trackerPanel->init();
	trackerPanel->setPreferredSize(0, 24);
	trackerPanel->setPercentageBuffered(.75);
	trackerPanel->setTrackerPercentage(.25);
	layout->addComponent(trackerPanel, &layoutProps);

	// Add Control Panel to layout
	seekPanel->init();
	seekPanel->setPreferredSize(0, 32);
	layout->addComponent(seekPanel);
}