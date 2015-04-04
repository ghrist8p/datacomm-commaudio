/*-----------------------------------------------------------------------------
-- SOURCE FILE: MicReader.h - This file provides functionality for reading
-- microphone input data into a MessageQueue.
--
-- PUBLIC FUNCTIONS:
-- static MicReader *getInstance();
-- MicReader(int sampleRate, float intervalLength, MessageQueue *queue, HWND owner);
-- void startReading();
-- void stopReading();
-- static size_t calculateBufferSize(int sampleRate, float intervalLength);
--
-- DATE: April 4, 2015
--
-- REVISIONS:
--
-- DESIGNER: Calvin Rempel
--
-- PROGRAMMER: Calvin Rempel
--
-- NOTES:
-- This class uses the Win32 waveform functions to read data from the
-- microphone and feed it into a MessageQueue.
-----------------------------------------------------------------------------*/

#ifndef _MIC_READER_H_
#define _MIC_READER_H_

#include "../common.h"
#include <mmsystem.h>
#include <queue>

#define WM_MIC_STOPPED_READING (WM_USER + 0x0001)
#define MIC_INPUT_MQUEUE_TYPE 101
#define MIC_BITS_PER_SAMPLE 8
#define NUM_MIC_CHANNELS 2

class MessageQueue;

/*-----------------------------------------------------------------------------
-- CLASS: MicReader
--
-- DESCRIPTION: This class provides functionality for reading Microphone
-- input data into a MessageQueue. It contains a static "getInstance" method
-- which will return the PREVIOUSLY instantiated instance. This means it is
-- ideal to create a single instance ONCE at the beginning of the program and
-- then use getInstance afterwards to get access to the created instance
-- elsewhere in the application.
-----------------------------------------------------------------------------*/
class MicReader
{
public:
	/* CONSTRUCTORS/DESTRUCTORS */
	MicReader(int sampleRate, float intervalLength, MessageQueue *queue, HWND owner);
	~MicReader(){};

	/* PUBLIC MEMBER METHODS */
	void startReading();
	void stopReading();

	/* PUBLIC STATIC MEMBER METHODS */
	static MicReader *getInstance();
	static size_t calculateBufferSize(int sampleRate, float intervalLength);

private:
	/* PRIVATE MEMBER METHODS */
	void addBuffer();
	void readIn();

	/* PRIVATE STATIC MEMBER METHODS */
	static void CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);

	/* PRIVATE STATIC MEMBER DATA */
	static MicReader *instance;

	/* PRIVATE MEMBER DATA */
	HWAVEIN mic;
	HWND owner;
	MessageQueue *mqueue;
	MMRESULT result;
	WAVEFORMATEX format;

	bool recording;
	float recordLength;
	int sampleRate;
	int buffLen;
	std::queue<WAVEHDR*> wavHeaders;
};

#endif
