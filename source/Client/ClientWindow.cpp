#include "ClientWindow.h"

#include "resource.h"

#include "../GuiLibrary/GuiPanel.h"
#include "../GuiLibrary/GuiStatusBar.h"
#include "../GuiLibrary/GuiLinearLayout.h"
#include "../GuiLibrary/GuiButton.h"
#include "../GuiLibrary/GuiLabel.h"
#include "../GuiLibrary/GuiScrollList.h"
#include "../GuiLibrary/GuiScrollBar.h"
#include "../GuiLibrary/GuiTextBox.h"
#include "PlaybackTrackerPanel.h"
#include "ButtonPanel.h"
#include "FileListItem.h"
#include "ConnectionWindow.h"
#include "../Buffer/MessageQueue.h"
#include "../Buffer/JitterBuffer.h"
#include "ReceiveThread.h"
#include "VoiceBufferer.h"
#include "MicReader.h"
#include "PlayWave.h"
#include "MusicBufferer.h"
#include "MusicReader.h"
#include "MusicBuffer.h"
#include "ClientControlThread.h"

ClientWindow* ClientWindow::curClientWindow = 0;

ClientWindow::ClientWindow(HINSTANCE hInst)
	: GuiWindow(hInst)
{
	setup(windowClass, NULL, windowStyles);
	setMiniumSize(700, 325);
	setExitOnClose(true);

	playButtonUp = LoadBitmap(hInst, L"IMG_PLAY_BUTTON_UP");
	playButtonDown = LoadBitmap(hInst, L"IMG_PLAY_BUTTON_DOWN");

	stopButtonUp = LoadBitmap(hInst, L"IMG_STOP_BUTTON_UP");
	stopButtonDown = LoadBitmap(hInst, L"IMG_STOP_BUTTON_DOWN");

	darkBackground = (HBRUSH)CreateSolidBrush(RGB(15, 15, 15));
	lightBackground = (HBRUSH)CreateSolidBrush(RGB(30, 30, 30));;
	accentBrush = (HBRUSH)CreateSolidBrush(RGB(0, 162, 232));
	nullPen = (HPEN)CreatePen(PS_SOLID, 0, 0);
	borderPen = (HPEN)CreatePen(PS_SOLID, 1, RGB(128, 0, 128));

    voiceTargetAddress[0] = 0;

	recording = false;
	requestingRecorderStop = false;
	micMQueue = new MessageQueue(1000,AUDIO_BUFFER_LENGTH);

	curClientWindow = this;
}

DWORD WINAPI ClientWindow::MicThread(LPVOID lpParameter)
{
	ClientWindow* This = (ClientWindow*)lpParameter;
	return This->ThreadStart();
}

DWORD ClientWindow::ThreadStart(void)
{
	int useless;
	int length;

	voicePacket.index = 0;

	// continuously send voice data over the network when it becomes available
	while (true)
	{
		++(voicePacket.index);
		micMQueue->dequeue(&useless, voicePacket.data, &length);
        udpSock->Send(MICSTREAM,&voicePacket,sizeof(voicePacket),voiceTargetAddress,MULTICAST_PORT);
	}
}

ClientWindow::~ClientWindow()
{
	delete topPanel;
	delete topPanelStretch;
	delete fileContainerPanel;
	delete seekPanel;
	delete micTargetButton;
	delete statusBar;
	delete trackerPanel;
	delete buttonSpacer1;
	delete buttonSpacer2;
	delete bottomSpacer;
	delete playButton;
	delete stopButton;
	delete voiceTargetInput;
	//delete voiceTargetLabel;

	DeleteObject(playButtonUp);
	DeleteObject(playButtonDown);
	DeleteObject(stopButtonUp);
	DeleteObject(stopButtonDown);
	DeleteObject(nullPen);
	DeleteObject(darkBackground);
	DeleteObject(lightBackground);
	DeleteObject(accentBrush);
	DeleteObject(borderPen);
}

void ClientWindow::addRemoteFile(SongName song)
{
	fileContainerPanel->addItem(new FileListItem(fileContainerPanel, this, hInst, song));
}

void ClientWindow::onCreate()
{
	setTitle(L"CommAudio Client");
	setSize(700, 325);
	micReader = new MicReader(AUDIO_SAMPLE_RATE, AUDIO_BUFFER_LENGTH, micMQueue, getHWND());
	this->addMessageListener(WM_MIC_STOPPED_READING, onMicStop, this);

	// Create Elements
	topPanel = new GuiPanel(hInst, this);
	topPanelStretch = new GuiPanel(hInst, topPanel);
	fileContainerPanel = new GuiScrollList(hInst, this);
	seekPanel = new GuiPanel(hInst, this);
	//voiceTargetLabel = new GuiLabel(hInst, topPanel);
	voiceTargetInput = new GuiTextBox(hInst, topPanel, false);
	micTargetButton = new GuiButton(hInst, topPanel, IDB_MIC_TOGGLE);
	statusBar = new GuiStatusBar(hInst, this);
	trackerPanel = new PlaybackTrackerPanel(hInst, this);
	playButton = new ButtonPanel(hInst, seekPanel, playButtonUp, playButtonDown);
	stopButton = new ButtonPanel(hInst, seekPanel, stopButtonUp, stopButtonDown);
	buttonSpacer1 = new GuiPanel(hInst, seekPanel);
	buttonSpacer2 = new GuiPanel(hInst, seekPanel);
	bottomSpacer = new GuiPanel(hInst, this);

	// Create Layouts
	GuiLinearLayout *layout = (GuiLinearLayout*) this->getLayoutManager();
	GuiLinearLayout::Properties layoutProps;
	layout->zeroProperties(&layoutProps);
	layout->setHorizontal(false);

	// Add Top Panel to Layout
	topPanel->init();
	topPanel->enableCustomDrawing(true);
	topPanel->setBorderPen(nullPen);
	topPanel->setBackgroundBrush(darkBackground);
	topPanel->setPreferredSize(0, 32);
	layout->addComponent(topPanel);

	// Add File Panel to layout
	fileContainerPanel->init();
	fileContainerPanel->enableCustomDrawing(true);
	fileContainerPanel->setBorderPen(nullPen);
	fileContainerPanel->setBackgroundBrush(lightBackground);
	fileContainerPanel->getScrollBar()->setColours(NULL, NULL, NULL, accentBrush);
	layoutProps.weight = 1;
	fileContainerPanel->setPreferredSize(0, 0);
	layout->addComponent(fileContainerPanel, &layoutProps);

	// Add Tracker Panel to layout
	trackerPanel->init();
	trackerPanel->setPreferredSize(0, 24);
	trackerPanel->setPercentageBuffered(.75);
	trackerPanel->setTrackerPercentage(.25);
	trackerPanel->setBorderPen(nullPen);
	trackerPanel->addMessageListener(WM_SEEK, ClientWindow::onSeek, this);
	layout->addComponent(trackerPanel, &layoutProps);

	// Add Control Panel to layout
	seekPanel->init();
	seekPanel->setPreferredSize(0, 64);
	seekPanel->enableCustomDrawing(true);
	seekPanel->setBorderPen(nullPen);
	seekPanel->setBackgroundBrush(darkBackground);
	layout->addComponent(seekPanel);

	// Add bottom spacer
	bottomSpacer->init();
	bottomSpacer->setPreferredSize(0, 8);
	bottomSpacer->enableCustomDrawing(true);
	bottomSpacer->setBorderPen(nullPen);
	bottomSpacer->setBackgroundBrush(darkBackground);
	layout->addComponent(bottomSpacer);

	// Add Top Spacer
	layout = (GuiLinearLayout*)topPanel->getLayoutManager();
	layout->setHorizontal(true);
	topPanelStretch->init();
	topPanelStretch->setPreferredSize(300, 0);
	topPanelStretch->enableCustomDrawing(true);
	topPanelStretch->setBackgroundBrush(darkBackground);
	topPanelStretch->setBorderPen(nullPen);
	layout->addComponent(topPanelStretch);

	// Add Microphone Label
	//voiceTargetLabel->init();
	//voiceTargetLabel->setText(L"Target");
	//layout->addComponent(voiceTargetLabel);

	// Add Microphone Textbox
	voiceTargetInput->init();
	voiceTargetInput->enableCustomDrawing(false);
	voiceTargetInput->setPreferredSize(200, 28);
	layout->addComponent(voiceTargetInput);

	// Add Microphone Button
	micTargetButton->init();
	micTargetButton->setText(L"Start Speaking");
	layout->addComponent(micTargetButton);
	topPanel->addCommandListener(BN_CLICKED, onClickMic, this);

	// Create Play Button
	playButton->init();
	playButton->setClickListener(ClientWindow::onClickPlay);
	playButton->setPreferredSize(64, 64);
	playButton->enableCustomDrawing(true);
	playButton->setBackgroundBrush(darkBackground);
	playButton->setBorderPen(nullPen);

	// Create Stop Button
	stopButton->init();
	stopButton->setClickListener(ClientWindow::onClickStop);
	stopButton->setPreferredSize(64, 64);
	stopButton->enableCustomDrawing(true);
	stopButton->setBackgroundBrush(darkBackground);
	stopButton->setBorderPen(nullPen);

	// Create Button Spacers
	buttonSpacer1->init();
	buttonSpacer1->enableCustomDrawing(true);
	buttonSpacer1->setBorderPen(nullPen);
	buttonSpacer1->setBackgroundBrush(darkBackground);

	buttonSpacer2->init();
	buttonSpacer2->enableCustomDrawing(true);
	buttonSpacer2->setBorderPen(nullPen);
	buttonSpacer2->setBackgroundBrush(darkBackground);

	// Add Play Button In Center
	layout = (GuiLinearLayout*)seekPanel->getLayoutManager();
	layout->setHorizontal(true);

	layout->addComponent(buttonSpacer1);
	layout->addComponent(playButton);
	layout->addComponent(stopButton);
	layout->addComponent(buttonSpacer2);

    // create all the buffers and stuff
	JitterBuffer* musicJitBuf = new JitterBuffer(5000,100,AUDIO_BUFFER_LENGTH,50,0);
	
	q1 = new MessageQueue(100,sizeof(LocalDataPacket));
	udpSock = new UDPSocket(MULTICAST_PORT,q1);
	udpSock->setGroup(MULTICAST_ADDR,1);
	ReceiveThread* recvThread = new ReceiveThread(musicJitBuf,q1);
	recvThread->start();
	

	ClientControlThread * cct = ClientControlThread::getInstance();
	cct->setClientWindow( this );

	MessageQueue* q2 = new MessageQueue(100,AUDIO_BUFFER_LENGTH);
	musicPlayer = new PlayWave(200,q2);
	musicfile = new MusicBuffer(trackerPanel, musicPlayer);	
	MusicBufferer* musicbuf = new MusicBufferer(musicJitBuf, musicfile);
	MusicReader* mreader = new MusicReader(q2, musicfile);
	
	musicPlayer->setVolume(0);
	musicPlayer->startPlaying(AUDIO_SAMPLE_RATE, AUDIO_BITS_PER_SAMPLE, NUM_AUDIO_CHANNELS);

     DWORD useless;
	CreateThread(NULL, 0, MicThread, (void*)this, 0, &useless);
	
}

void ClientWindow::startConnection()
{
	musicPlayer->setVolume(0xFFFF);
}

void ClientWindow::onClickPlay(void*)
{
	curClientWindow->musicfile->resumeEnqueue();
	curClientWindow->musicPlayer->resumePlaying();
}
void ClientWindow::onClickStop(void*)
{
	curClientWindow->musicfile->stopEnqueue();
	curClientWindow->musicPlayer->stopPlaying();
}

bool ClientWindow::onClickMic(GuiComponent *_pThis, UINT command, UINT id, WPARAM wParam, LPARAM lParam, INT_PTR *retval)
{
	ClientWindow *pThis = (ClientWindow*)_pThis;

	if (!pThis->requestingRecorderStop)
	{
		if (pThis->recording)
		{
			pThis->requestingRecorderStop = true;
			pThis->micReader->stopReading();
		}
		else
		{
			// get target address, and if it is invalid, bail out and show a
			// message box
			size_t useless;
			wcstombs_s(&useless,pThis->voiceTargetAddress,pThis->voiceTargetInput->getText(),STR_LEN);
			struct sockaddr_in destination;
			destination.sin_addr.s_addr = inet_addr(pThis->voiceTargetAddress);
			if (destination.sin_addr.s_addr == INADDR_NONE)
			{
				#ifdef DEBUG
				MessageBox(NULL, L"The target ip address entered must be a legal IPv4 address", L"ERROR", MB_ICONERROR);
				#endif
				return true;
			}

			// start recording
			pThis->micTargetButton->setText(L"Stop Speaking");
			pThis->recording = true;
			pThis->micReader->startReading();
		}
	}

	return true;
}

bool ClientWindow::onMicStop(GuiComponent *_pThis, UINT command, UINT id, WPARAM wParam, LPARAM lParam, INT_PTR *retval)
{
	ClientWindow *pThis = (ClientWindow*)_pThis;

	pThis->micTargetButton->setText(L"Start Speaking");
	pThis->recording = false;
	pThis->requestingRecorderStop = false;
	pThis->voicePacket.index = 0;

	return true;
}

bool ClientWindow::onSeek(GuiComponent *_pThis, UINT command, UINT id, WPARAM wParam, LPARAM lParam, INT_PTR *retval)
{
	ClientWindow *pThis = (ClientWindow*)_pThis;

	double percent = ((double)wParam) / 1000.0;

    pThis->musicfile->seekBuf(percent);

	return true;
}
