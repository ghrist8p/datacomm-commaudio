#ifndef MUSICREADER_H
#define MUSICREADER_H

#include "../Common.h"

class MusicBufferer;
class MessageQueue;
class MusicBuffer;

class MusicReader
{
private:
	MusicBuffer* musicbuffer;
	MessageQueue* msgqueue;

public:
	MusicReader(MessageQueue* mqueue, MusicBuffer* mbuffer);
	~MusicReader();
	static DWORD WINAPI fileThread(LPVOID lpParameter);
	DWORD ThreadStart(void);

};

#endif