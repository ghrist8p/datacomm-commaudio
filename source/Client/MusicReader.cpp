/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: MusicReader.cpp
--
-- FUNCTIONS:
	MusicReader(MessageQueue* mqueue, MusicBuffer* mbuffer);
	~MusicReader();
	static DWORD WINAPI fileThread(LPVOID lpParameter);
	DWORD ThreadStart(void);
--
-- DATE: April 5, 2015
--
-- REVISIONS: --
--
-- DESIGNER: Manuel Gonzales
--
-- PROGRAMMER: Manuel Gonzales
--
-- NOTES:
-- This is the file containing all the necessary functions to Read from the Music buffer and pass it to the 
-- messageQeuue to play it.
----------------------------------------------------------------------------------------------------------------------*/

#include "MusicReader.h"
#include "MusicBuffer.h"
#include "../Buffer/MessageQueue.h"

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: MusicReader
--
-- DATE: April 5, 2015
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Manuel Gonzales
--
-- PROGRAMMER: Manuel Gonzales
--
-- INTERFACE: MusicReader::MusicReader(MessageQueue* mqueue, MusicBuffer* mbuffer) : msgqueue(mqueue)
--
--  mqueue : message queue to use to store the data.
--	mbuffer : soruce buffer to get data
--
--	RETURNS: nothing.
--
--	NOTES:
--  This is the constructor for the Music Reader it will create the thread to start reading music from the buffer.
----------------------------------------------------------------------------------------------------------------------*/
MusicReader::MusicReader(MessageQueue* mqueue, MusicBuffer* mbuffer) : msgqueue(mqueue)
{
	musicbuffer = mbuffer;
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
-- FUNCTION: MusicReader
--
-- DATE: April 5 2015
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Manuel Gonzales
--
-- PROGRAMMER: Manuel Gonzales
--
-- INTERFACE: MusicReader::~MusicReader()
--
--	RETURNS: nothing.
--
--	NOTES:
--  This is the destructor that will do the cleanup.
----------------------------------------------------------------------------------------------------------------------*/
MusicReader::~MusicReader()
{
	//delete msgqueue
	//fclose(music_file);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: fileThread
--
-- DATE: April 5, 2015
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Manuel Gonzales
--
-- PROGRAMMER: Manuel Gonzales
--
-- INTERFACE: DWORD WINAPI MusicReader::fileThread(LPVOID lpParameter)
--
--	lpParameter : instance of the object
--
--	RETURNS: nothing.
--
--	NOTES:
--  This will call the start Thread fucntion
----------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI MusicReader::fileThread(LPVOID lpParameter)
{
	MusicReader* This = (MusicReader*)lpParameter;
	return This->ThreadStart();
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: ThreadStart
--
-- DATE: April 5, 2015
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Manuel Gonzales
--
-- PROGRAMMER: Manuel Gonzales
--
-- INTERFACE: DWORD MusicReader::ThreadStart(void)
--
--	RETURNS: FALSE in error.
--
--	NOTES:
--  This function will start getting data from the music buffer and will send it to the message queue
----------------------------------------------------------------------------------------------------------------------*/
DWORD MusicReader::ThreadStart(void)
{
	//music_file = fopen("tempmusic.txt", "rb");
	int ret;
	int len;
	len = msgqueue->elementSize;
	char* music_data = (char*)malloc(sizeof(char) * len);
	
	while (true)
	{
		ret = musicbuffer->readBuf(music_data, len);

		if (ret)
		{
			msgqueue->enqueue(ACTUAL_MUSIC, music_data, len);
		}
	}

	return true;
}