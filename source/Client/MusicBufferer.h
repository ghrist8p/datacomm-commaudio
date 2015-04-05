#ifndef MUSICBUFFERER_H
#define MUSICBUFFERER_H

#include "../Buffer/JitterBuffer.h"
#include "../Common.h"

class MusicBufferer
{
private:
	JitterBuffer* music_buffer;
	FILE* musicfile;

public:
	MusicBufferer(JitterBuffer* musicJB);
	~MusicBufferer();
	void clearBuffer();
	static DWORD WINAPI fileThread(LPVOID lpParameter);
	DWORD ThreadStart(void);

};

#endif