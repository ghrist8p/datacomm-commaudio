#ifndef MUSICBUFFERER_H
#define MUSICBUFFERER_H

#include "../Buffer/JitterBuffer.h"
#include "../Common.h"

class MusicBuffer;
class MessageQueue;

class MusicBufferer
{

private:
	int elementSize;
	JitterBuffer* music_jitter;
	MusicBuffer* music_buffer;
	MessageQueue* mqueue;
	static DWORD WINAPI fileThread(LPVOID lpParameter);
	DWORD ThreadStart(void);

public:
	MusicBufferer(JitterBuffer* musicJB, MusicBuffer* musicB);
	~MusicBufferer();
	void clearBuffer();
	int getSize();
};

#endif