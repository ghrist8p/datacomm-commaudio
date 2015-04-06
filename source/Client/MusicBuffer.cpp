/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: MusicBuffer.cpp
--
-- FUNCTIONS:
	MusicBuffer();
	~MusicBuffer();
	void writeBuf(char* data, int len);
	void readBuf(char* data, int len);
	void seekBuf(long index);
	void newSong();
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
-- This is the file containing all the necessary functions to make use of a music buffer. It has fucntions to read
--	and write into it as well as semaphores to control the flow of data.
----------------------------------------------------------------------------------------------------------------------*/

#include "MusicBuffer.h"

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: MusicBuffer
--
-- DATE: April 5, 2015
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Manuel Gonzales
--
-- PROGRAMMER: Manuel Gonzales
--
-- INTERFACE: MusicBuffer::MusicBuffer()
--
--	RETURNS: nothing.
--
--	NOTES:
--  This is the constructor for the Music Reader it will allocate memory for the buffer and will instatiate the semaphore
--	and mutex.
----------------------------------------------------------------------------------------------------------------------*/
MusicBuffer::MusicBuffer()
{
	buffer = (char*)malloc(sizeof(char) * SUPERSIZEBUF);
	writeindex = 0;
	readindex = 0;

	canRead = CreateSemaphore(NULL, 0, SUPERSIZEBUF, NULL);
	mutexx = CreateMutex(NULL, FALSE, NULL);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: MusicBuffer
--
-- DATE: April 5, 2015
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Manuel Gonzales
--
-- PROGRAMMER: Manuel Gonzales
--
-- INTERFACE: MusicBuffer::~MusicBuffer()
--
--	RETURNS: nothing.
--
--	NOTES:
--  This is the destructor that will do the cleanup.
----------------------------------------------------------------------------------------------------------------------*/
MusicBuffer::~MusicBuffer()
{
	free(buffer);
	CloseHandle(canRead);
	CloseHandle(mutexx);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: writeBuf
--
-- DATE: April 5, 2015
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Manuel Gonzales
--
-- PROGRAMMER: Manuel Gonzales
--
-- INTERFACE: void MusicBuffer::writeBuf(char* data, int len)
--
--  data : data to store in buffer
--	len : length of data in bytes
--
--	RETURNS: nothing.
--
--	NOTES:
--  This function will write the data into the buffer. it is guarded by a mutex.
----------------------------------------------------------------------------------------------------------------------*/
void MusicBuffer::writeBuf(char* data, int len)
{
	unsigned long wdifference;

	WaitForSingleObject(mutexx, INFINITE);

	if (writeindex + len > SUPERSIZEBUF)
	{
		wdifference = writeindex + len - SUPERSIZEBUF - 1;
		memcpy(buffer + writeindex, data, wdifference);
		writeindex = 0;
		memcpy(buffer + writeindex, data + wdifference, len - wdifference);
		writeindex = len - wdifference;
	}
	else
	{
		memcpy(buffer + writeindex, data, len);
		writeindex += len;
	}

	ReleaseMutex(mutexx);
	ReleaseSemaphore(canRead, 1, NULL);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: readBuf
--
-- DATE: April 5, 2015
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Manuel Gonzales
--
-- PROGRAMMER: Manuel Gonzales
--
-- INTERFACE: void MusicBuffer::readBuf(char* data, int len)
--
--  data : pointer to location to store the data
--	len : length of data to be read in bytes
--
--	RETURNS: nothing.
--
--	NOTES:
--  This function will read the data into the pointer passed. it is guarded by a mutex and a semaphore.
----------------------------------------------------------------------------------------------------------------------*/
void MusicBuffer::readBuf(char* data, int len)
{
	unsigned long rdifference;
	WaitForSingleObject(canRead, INFINITE);
	WaitForSingleObject(mutexx, INFINITE);

	if (readindex + len > SUPERSIZEBUF)
	{
		rdifference = readindex + len - SUPERSIZEBUF - 1;
		memcpy(data, buffer + readindex, rdifference);
		readindex = 0;
		memcpy(data + rdifference, buffer + readindex, len - rdifference);
		writeindex = len - rdifference;
	}
	else
	{
		memcpy(data, buffer + readindex, len);
		readindex += len;
	}

	ReleaseMutex(mutexx);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: seekBuf
--
-- DATE: April 5, 2015
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Manuel Gonzales
--
-- PROGRAMMER: Manuel Gonzales
--
-- INTERFACE: void MusicBuffer::seekBuf(long index)
--
--  index : location to read form the buffer 
--
--	RETURNS: nothing.
--
--	NOTES:
--  This function will set the current readindex to the desired one.
----------------------------------------------------------------------------------------------------------------------*/
void MusicBuffer::seekBuf(long index)
{
	WaitForSingleObject(mutexx, INFINITE);

	readindex = index;

	ReleaseMutex(mutexx);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: newSong
--
-- DATE: April 5, 2015
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Manuel Gonzales
--
-- PROGRAMMER: Manuel Gonzales
--
-- INTERFACE: void MusicBuffer::newSong()
--
--	RETURNS: nothing.
--
--	NOTES:
--  This function will set the current read index to match the write index. This means a new song has started
--  and it should stop reading data form the old one.
----------------------------------------------------------------------------------------------------------------------*/
void MusicBuffer::newSong()
{
	WaitForSingleObject(mutexx, INFINITE);

	readindex = writeindex;

	ReleaseMutex(mutexx);
}