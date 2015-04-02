#ifndef _MIC_READER_H_
#define _MIC_READER_H_

#include <Windows.h>
#include <mmsystem.h>

class MicReader
{
public:
	MicReader(int numSeconds);
	~MicReader();

	void startReading();
	void stopReading();

	char *getRecordedData();

private:
	HWAVEIN mic;
	WAVEHDR wavHeader;

	int recordLength;
	int sampleRate;
	int buffLen; // 10 seconds worth of buffer
	short int *waveIn;
	MMRESULT result;
	WAVEFORMATEX format;
};

#endif