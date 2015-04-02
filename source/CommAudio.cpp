#define DEBUG
#ifndef DEBUG
/*-------------------------------------------------------------------------------------------------
-- SOURCE FILE: CommAudio.cpp - An application that implements a client/server model where
--				mulitple clients may connect to the server and audio is multicast from the server
--				to the connected clients.
--
-- PROGRAM: CommAudio
--
-- FUNCTIONS:
-- int CALLBACK WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
--
--
-- DATE: February 25, 2015
--
-- REVISIONS:
--
-- DESIGNER: Calvin Rempel
--
-- PROGRAMMER: Calvin Rempel
--
-- NOTES:
-- The program implements both client and server applications where multiple clients are able to
-- connect to the server, select an audio file, and have that audio file multicasted to, and
-- played by, all connected clients. Clients can also request file transfer of the audio file to
-- store the file locally. Additionally, clients may send their microphone input to another
-- connected client which will play the results.
-------------------------------------------------------------------------------------------------*/

#define APP_SERVER 0
#define APP_CLIENT 1

// Set to APP_SERVER to build the Server, APP_CLIENT to build the client.
#define BUILD_TARGET APP_CLIENT

#include "Server/ServerWindow.h"
#include "Client/ConnectionWindow.h"
#include "Client/MicReader.h"
#include "Client/PlayWave.h"
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "MSIMG32.lib")
#pragma comment(lib, "Gdi32.lib")
#pragma comment(lib, "Winmm.lib")

// The version of Windows Sockets required.
const DWORD WSA_VERSION = 0x0202;

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: WinMain
--
-- DATE: February 25, 2015
--
-- REVISIONS:
--
-- DESIGNER: Calvin Rempel
--
-- PROGRAMMER: Calvin Rempel
--
-- INTERFACE: int CALLBACK WinMain(HINSTANCE hInst, HINSTANCE hPrevInst,
--								   LPSTR lpCmdLine, int nCmdShow)
--
-- RETURNS: int - The exit status of the program.
--
-- NOTES:
-- This is the main entry point to the application. If BUILD_TARGET is set to APP_SERVER then
-- the server application will be built. If BUILD_TARGET is set to APP_CLIENT then the client
-- application will be built.
-------------------------------------------------------------------------------------------------*/
int CALLBACK WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
	MSG msg;
	GuiWindow *window;

	// Initialize Winsock
	WSADATA wsaData;
	INT error;

	if ((error = WSAStartup(WSA_VERSION, &wsaData)) != 0)
	{
		MessageBox(NULL, L"WSAStartup Error", L"Fatal Error", MB_ICONERROR);
		WSACleanup();
		return 1;
	}

	// Create the Server or Client based on preprocessor defines
	#if BUILD_TARGET == APP_SERVER
		window = new ServerWindow(hInst);
	#else
		MicReader micReader(5);
		micReader.startReading();
		char *recordedData = micReader.getRecordedData();
		PlayWave player;
		player.playWave(recordedData);
		window = new ConnectionWindow(hInst);
	#endif

	// Ensure the Window was successfully created
	if (!window)
	{
		MessageBox(NULL, L"Failed to Initialize Window", L"Fatal Error", MB_ICONERROR);
		return 1;
	}

	// Initialize the Window and make it visible.
	window->init();
	window->setVisible(true);

	// Process Window Messages
	while (GetMessage(&msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	delete window;
	return 0;
}
#endif
