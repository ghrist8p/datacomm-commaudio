#include "MusicBuffer.h"

MusicBuffer::MusicBuffer()
{
	buffer = (char*)malloc(sizeof(char) * SUPERSIZEBUF);
	writeindex = 0;
	readindex = 0;

	canRead = CreateSemaphore(NULL, 0, SUPERSIZEBUF, NULL);
	mutexx = CreateMutex(NULL, FALSE, NULL);
}

MusicBuffer::~MusicBuffer()
{
	free(buffer);
	CloseHandle(canRead);
	CloseHandle(mutexx);
}

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

void MusicBuffer::seekBuf(long index)
{
	WaitForSingleObject(mutexx, INFINITE);

	readindex = index;

	ReleaseMutex(mutexx);
}

void MusicBuffer::newSong()
{
	WaitForSingleObject(mutexx, INFINITE);

	readindex = writeindex;

	ReleaseMutex(mutexx);
}