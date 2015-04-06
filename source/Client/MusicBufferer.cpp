#include "MusicBufferer.h"
#include "MusicBuffer.h"

MusicBufferer::MusicBufferer(JitterBuffer* musicJB, MusicBuffer* mbuffer) : music_jitter(musicJB)
{
	elementSize = music_jitter->getElementSize();
	music_buffer = mbuffer;

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
	//delete music_buffer;
	//fclose(musicfile);
}

void MusicBufferer::clearBuffer()
{
	//musicfile = fopen("tempmusic.txt", "wb");
	//fseek(musicfile, 0, SEEK_SET);

}

DWORD WINAPI MusicBufferer::fileThread(LPVOID lpParameter)
{
	MusicBufferer* This = (MusicBufferer*)lpParameter;
	return This->ThreadStart();
}

DWORD MusicBufferer::ThreadStart(void)
{
	//musicfile = fopen("tempmusic.txt", "wb");	
	char* music_data = (char*) malloc (sizeof(char) * elementSize);


	while(true)
	{
		music_jitter->get(music_data);
		//fwrite(music_data, 1, elementSize, musicfile);
		music_buffer->writeBuf(music_data, elementSize);		
	}
}

int MusicBufferer::getSize()
{
	return elementSize;
}