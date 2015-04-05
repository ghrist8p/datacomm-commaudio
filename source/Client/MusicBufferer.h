#ifndef MUSICBUFFERER_H
#define MUSICBUFFERER_H

#include "../Buffer/JitterBuffer.h"
#include "../Common.h"

class MusicBufferer
{

private:
	int elementSize;
	JitterBuffer* music_buffer;
	FILE* musicfile;
	static DWORD WINAPI fileThread(LPVOID lpParameter);
	DWORD ThreadStart(void);

public:
	MusicBufferer(JitterBuffer* musicJB);
	~MusicBufferer();
	void clearBuffer();
	int getSize();	

};

#endif