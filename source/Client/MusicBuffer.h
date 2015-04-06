#include "../Common.h"

#define SUPERSIZEBUF	99999999

class MusicBuffer
{
private:
	char* buffer;
	unsigned long writeindex;
	unsigned long readindex;

	HANDLE canRead;
	HANDLE mutexx;

public:
	MusicBuffer();
	~MusicBuffer();
	void writeBuf(char* data, int len);
	void readBuf(char* data, int len);
	void seekBuf(long index);
	void newSong();
};