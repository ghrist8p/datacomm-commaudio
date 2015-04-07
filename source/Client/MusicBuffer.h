#include "../Common.h"

#define SUPERSIZEBUF	99999999

class PlaybackTrackerPanel;

class MusicBuffer
{
private:
	char* buffer;
	unsigned long writeindex;
	unsigned long readindex;
	unsigned long currentsong_size;
	unsigned long song_startindex;

	PlaybackTrackerPanel* TrackerPanel;
	HANDLE canRead;
	HANDLE mutexx;

public:
	MusicBuffer(PlaybackTrackerPanel* TrackerP);
	~MusicBuffer();
	void writeBuf(char* data, int len);
	void readBuf(char* data, int len);
	void seekBuf(double percentage);
	void newSong(unsigned long song_size);
};