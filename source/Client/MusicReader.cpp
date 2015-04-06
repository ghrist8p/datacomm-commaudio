#include "MusicReader.h"
#include "MusicBuffer.h"
#include "../Buffer/MessageQueue.h"

MusicReader::MusicReader(MessageQueue* mqueue, MusicBuffer* mbuffer) : msgqueue(mqueue)
{
	musicbuffer = mbuffer;
	HANDLE ThreadHandle;
	DWORD ThreadId;

	if ((ThreadHandle = CreateThread(NULL, 0, fileThread, (void*)this, 0, &ThreadId)) == NULL)
	{
		MessageBox(NULL, L"CreateThread failed with error", L"ERROR", MB_ICONERROR);
		return;
	}
}
MusicReader::~MusicReader()
{
	//delete msgqueue
	//fclose(music_file);
}

DWORD WINAPI MusicReader::fileThread(LPVOID lpParameter)
{
	MusicReader* This = (MusicReader*)lpParameter;
	return This->ThreadStart();
}

DWORD MusicReader::ThreadStart(void)
{
	//music_file = fopen("tempmusic.txt", "rb");
	int len;
	len = msgqueue->elementSize;
	char* music_data = (char*)malloc(sizeof(char) * len);
	
	while (true)
	{
		musicbuffer->readBuf(music_data, len);
		msgqueue->enqueue(ACTUAL_MUSIC, music_data, len);
	}

	return true;
}