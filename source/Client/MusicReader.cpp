#include "MusicReader.h"
#include "MusicBufferer.h"
#include "../Buffer/MessageQueue.h"

MusicReader::MusicReader(MusicBufferer* musicB, MessageQueue* mqueue) : msgqueue(mqueue)
{
	musicbuffer = musicB;

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
	delete musicbuffer;
	//fclose(music_file);
}

void MusicReader::restartBuffer()
{
	fseek(music_file, 0, SEEK_SET);
}

DWORD WINAPI MusicReader::fileThread(LPVOID lpParameter)
{
	MusicReader* This = (MusicReader*)lpParameter;
	return This->ThreadStart();
}

DWORD MusicReader::ThreadStart(void)
{
	music_file = fopen("tempmusic.txt", "rb");
	int len = 0;
	len = musicbuffer->getSize();
	char* music_data = (char*)malloc(sizeof(char) * len);

	while (true)
	{
		fread(music_data, 1, len, music_file);
		msgqueue->enqueue(ACTUAL_MUSIC, music_data, len);
	}
}