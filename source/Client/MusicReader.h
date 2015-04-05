#ifndef MUSICREADER_H
#define MUSICREADER_H

#include "../Common.h"

class MusicBufferer;
class MessageQueue;

class MusicReader
{
private:
	MusicBufferer* musicbuffer;
	FILE* music_file;
	MessageQueue* msgqueue;

public:
	MusicReader(MusicBufferer* musicB, MessageQueue* mqueue);
	~MusicReader();
	void restartBuffer();
	static DWORD WINAPI fileThread(LPVOID lpParameter);
	DWORD ThreadStart(void);

};

#endif