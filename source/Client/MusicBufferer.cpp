/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: Musicbufferer.cpp
--
-- FUNCTIONS:
	MusicBufferer(JitterBuffer* musicJB, MusicBuffer* musicB);
	~MusicBufferer();
	static DWORD WINAPI fileThread(LPVOID lpParameter);
	DWORD ThreadStart(void);	
	void clearBuffer();
	int getSize();
--
-- DATE: April 4, 2015
--
-- REVISIONS: April 5, Added music buffer support instead of using a text file.
--
-- DESIGNER: Manuel Gonzales
--
-- PROGRAMMER: Manuel Gonzales
--
-- NOTES:
-- This is the file containing all the necessary functions to get data form the JitterBuffer and pass it into
-- the Music buffer.
----------------------------------------------------------------------------------------------------------------------*/
#include "MusicBufferer.h"
#include "MusicBuffer.h"
#include "../Buffer/MessageQueue.h"

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: MusicBufferer
--
-- DATE: April 4, 2015
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Manuel Gonzales
--
-- PROGRAMMER: Manuel Gonzales
--
-- INTERFACE: MusicBufferer::MusicBufferer(JitterBuffer* musicJB, MusicBuffer* mbuffer) : music_jitter(musicJB)
--
--  musicJB : buffer source to get the data.
--	mbuffer : buffer where to store the data
--
--	RETURNS: nothing.
--
--	NOTES:
--  This is the constructor for the MusicBuffer it will create the thread tot start reading music from the jitter buffer.
----------------------------------------------------------------------------------------------------------------------*/
MusicBufferer::MusicBufferer(JitterBuffer* musicJB, MusicBuffer* mbuffer) : music_jitter(musicJB)
{
	elementSize = music_jitter->getElementSize();
	music_buffer = mbuffer;

	HANDLE ThreadHandle;
	DWORD ThreadId;

	if ((ThreadHandle = CreateThread(NULL, 0, fileThread, (void*)this, 0, &ThreadId)) == NULL)
	{
		#ifdef DEBUG
		MessageBox(NULL, L"CreateThread failed with error", L"ERROR", MB_ICONERROR));
		#endif
		return;
	}
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: MusicBufferer
--
-- DATE: April 4, 2015
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Manuel Gonzales
--
-- PROGRAMMER: Manuel Gonzales
--
-- INTERFACE: MusicBufferer::~MusicBufferer()
--
--	RETURNS: nothing.
--
--	NOTES:
--  This is the destructor that will do the cleanup.
----------------------------------------------------------------------------------------------------------------------*/
MusicBufferer::~MusicBufferer()
{
	//delete music_buffer;
	//fclose(musicfile);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: clearBuffer
--
-- DATE: April 4, 2015
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Manuel Gonzales
--
-- PROGRAMMER: Manuel Gonzales
--
-- INTERFACE: void MusicBufferer::clearBuffer()
--
--	lpParameter : instance of the object
--
--	RETURNS: nothing.
--
--	NOTES:
--  This function will move the index on themusic buffer so the reader cna start reading from the new song.
----------------------------------------------------------------------------------------------------------------------*/
void MusicBufferer::clearBuffer()
{
	//musicfile = fopen("tempmusic.txt", "wb");
	//fseek(musicfile, 0, SEEK_SET);
	//music_buffer->newSong();
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: fileThread
--
-- DATE: April 4, 2015
--
-- REVISIONS: --
--
-- DESIGNER: Manuel Gonzales
--
-- PROGRAMMER: Manuel Gonzales
--
-- INTERFACE: DWORD WINAPI MusicBufferer::fileThread(LPVOID lpParameter)
--
--	lpParameter : instance of the object
--
--	RETURNS: nothing.
--
--	NOTES:
--  This will call the start Thread fucntion
----------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI MusicBufferer::fileThread(LPVOID lpParameter)
{
	MusicBufferer* This = (MusicBufferer*)lpParameter;
	return This->ThreadStart();
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: ThreadStart
--
-- DATE: April 4, 2015
--
-- REVISIONS: April 5 Added music buffer.
--
-- DESIGNER: Manuel Gonzales
--
-- PROGRAMMER: Manuel Gonzales
--
-- INTERFACE: DWORD MusicBufferer::ThreadStart(void)
--
--	RETURNS: FALSE in error.
--
--	NOTES:
--  This function will start getting data from the jitter buffer and will send it to the music buffer
----------------------------------------------------------------------------------------------------------------------*/
DWORD MusicBufferer::ThreadStart(void)
{
	//musicfile = fopen("tempmusic.txt", "wb");	
	char* music_data = (char*) malloc (sizeof(char) * elementSize);


	while(true)
	{
		WaitForSingleObject(music_jitter->canGet,INFINITE);
		music_jitter->get(music_data);
		music_buffer->writeBuf(music_data, elementSize);
	}
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: getSize
--
-- DATE: April 4, 2015
--
-- REVISIONS: --
--
-- DESIGNER: Manuel Gonzales
--
-- PROGRAMMER: Manuel Gonzales
--
-- INTERFACE: int MusicBufferer::getSize()
--
--	RETURNS: size of the elements being read from the jitter buffer.
--
--	NOTES:
--
----------------------------------------------------------------------------------------------------------------------*/
int MusicBufferer::getSize()
{
	return elementSize;
}