#ifndef _MIC_READER_H_
#define _MIC_READER_H_

#include <Windows.h>
#include <mmsystem.h>

#define WM_MIC_STOPPED_READING (WM_USER + 0x0001)
#define MIC_INPUT_MQUEUE_TYPE 101
#define BITS_PER_SAMPLE 8;

class MessageQueue;

class MicReader
{
public:
	MicReader(int sampleRate, int intervalLength, MessageQueue *queue, HWND owner);
	~MicReader();

	void startReading();
	void stopReading();

	static size_t calculateBufferSize(int sampleRate, int intervalLength);
private:
	static DWORD WINAPI RecordThread(LPVOID lpParam);

	bool recording;
	HWAVEIN mic;
	WAVEHDR wavHeader;

	HWND owner;
	MessageQueue *mqueue;
	int recordLength;
	int sampleRate;
	int buffLen;
	short int *waveIn;
	MMRESULT result;
	WAVEFORMATEX format;
};

#endif