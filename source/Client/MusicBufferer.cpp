#include "MusicBufferer.h"

MusicBufferer::MusicBufferer(JitterBuffer* musicJB)
{
	music_buffer = musicJB;

	HANDLE ThreadHandle;
	DWORD ThreadId;

	if ((ThreadHandle = CreateThread(NULL, 0, fileThread, (void*)this, 0, &ThreadId)) == NULL)
	{
		MessageBox(NULL, L"CreateThread failed with error", L"ERROR", MB_ICONERROR);
		return;
	}
}
MusicBufferer::~MusicBufferer()
{
	delete music_buffer;
}

void MusicBufferer::clearBuffer()
{
	musicfile = fopen("tempmusic.txt", "wb");
	fseek(musicfile, 0, SEEK_SET);
}

DWORD WINAPI MusicBufferer::fileThread(LPVOID lpParameter)
{
	MusicBufferer* This = (MusicBufferer*)lpParameter;
	return This->ThreadStart();
}

DWORD MusicBufferer::ThreadStart(void)
{
	musicfile = fopen("tempmusic.txt", "wb");
	int len;
	//music_buffer->getElementSize(&len);
	char* music_data = (char*) malloc (sizeof(char) * len);
	music_buffer->get(music_data);

	while(true)
	{
		fwrite(music_data, 1, len, musicfile);
	}


}