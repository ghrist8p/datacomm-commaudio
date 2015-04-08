/*-----------------------------------------------------------------------------
-- SOURCE FILE: MicReader.cpp - This file provides functionality for reading
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

#include "MicReader.h"
#include <iostream>

#include "../Buffer/MessageQueue.h"

MicReader *MicReader::instance = NULL;

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: getInstance
--
-- DATE: April 4, 2015
--
-- REVISIONS:
--
-- DESIGNER: Calvin Rempel
--
-- PROGRAMMER: Calvin Rempel
--
-- INTERFACE: MicReader *getInstance()
--
-- RETURNS: the last instantiated MicReader
--
-- NOTES:
-- This is a static function that provides access to the last created instance
-- of MicReader.
-------------------------------------------------------------------------------------------------*/
MicReader *MicReader::getInstance()
{
	return instance;
}

MicReader::MicReader(int sampleRate, int buffLen, MessageQueue *queue, HWND owner)
{
	instance = this;

	// Initialize data
	this->mqueue = queue;
	this->recordLength = buffLen / sampleRate;
	this->buffLen = buffLen;
	this->recording = false;
	this->owner = owner;

	// Create the WAV format for the MicReader
	result = 0;
	format.wFormatTag = WAVE_FORMAT_PCM;
	format.wBitsPerSample = AUDIO_BITS_PER_SAMPLE;
	format.nChannels = NUM_AUDIO_CHANNELS;
	format.nSamplesPerSec = sampleRate;
	format.nAvgBytesPerSec = format.nSamplesPerSec * format.nChannels * format.wBitsPerSample / AUDIO_BITS_PER_SAMPLE;
	format.nBlockAlign = format.nChannels * format.wBitsPerSample / AUDIO_BITS_PER_SAMPLE;
	format.cbSize = 0;
}

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: MicReader
--
-- DATE: April 4, 2015
--
-- REVISIONS:
--
-- DESIGNER: Calvin Rempel
--
-- PROGRAMMER: Calvin Rempel
--
-- INTERFACE: MicReader(int sampleRate, float intervalLength, MessageQueue *queue, HWND owner)
--		int sampleRate		 : the number of samples to record per second
--		float intervalLength : the number of seconds to record into a single buffer
--		MessageQueue *queue	 : the MessageQueue to write data into as it's available
--		HWND owner			 : the window that will receive the shutdown message.
--
-- NOTES: Create a new MicReader
-------------------------------------------------------------------------------------------------*/
MicReader::MicReader(int sampleRate, float intervalLength, MessageQueue *queue, HWND owner)
{
	instance = this;

	// Initialize data
	this->mqueue = queue;
	this->recordLength = intervalLength;
	this->buffLen = calculateBufferSize(sampleRate, intervalLength);
	this->recording = false;
	this->owner = owner;

	// Create the WAV format for the MicReader
	result = 0;
	format.wFormatTag = WAVE_FORMAT_PCM;
	format.wBitsPerSample = AUDIO_BITS_PER_SAMPLE;
	format.nChannels = NUM_AUDIO_CHANNELS;
	format.nSamplesPerSec = sampleRate;
	format.nAvgBytesPerSec = format.nSamplesPerSec * format.nChannels * format.wBitsPerSample / AUDIO_BITS_PER_SAMPLE;
	format.nBlockAlign = format.nChannels * format.wBitsPerSample / AUDIO_BITS_PER_SAMPLE;
	format.cbSize = 0;
}

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: startReading
--
-- DATE: April 4, 2015
--
-- REVISIONS:
--
-- DESIGNER: Calvin Rempel
--
-- PROGRAMMER: Calvin Rempel
--
-- INTERFACE: startReading()
--
-- RETURNS: void
--
-- NOTES: This function starts reading from the microphone. When data is availalble, it is placed
-- into the MicReaders MessageQueue.
-------------------------------------------------------------------------------------------------*/
void MicReader::startReading()
{
	if (!recording)
	{
		recording = true;
		DWORD threadId;
		readIn();
	}
}

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: stopReading
--
-- DATE: April 4, 2015
--
-- REVISIONS:
--
-- DESIGNER: Calvin Rempel
--
-- PROGRAMMER: Calvin Rempel
--
-- INTERFACE: stopReading()
--
-- RETURNS: void
--
-- NOTES: This function stops reading from the microphone.
-------------------------------------------------------------------------------------------------*/
void MicReader::stopReading()
{
	if (recording)
	{
		recording = false;

		// Mark all Buffers as Complete
		waveInReset(mic);

		// Wait for the device to close
		while (waveInClose(mic) == WAVERR_STILLPLAYING){}
	}
}

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: calculateBufferSize
--
-- DATE: April 4, 2015
--
-- REVISIONS:
--
-- DESIGNER: Calvin Rempel
--
-- PROGRAMMER: Calvin Rempel
--
-- INTERFACE: calculateBufferSize(int sampleRate, float intervalLength)
--		int sampleRate		 : the number of samples recorded per second
--		float intervalLength : the number of seconds to record into a single buffer
--
-- RETURNS: size_t
--
-- NOTES: This is a public static function that provides the buffer size, given the parameters.
-------------------------------------------------------------------------------------------------*/
size_t MicReader::calculateBufferSize(int sampleRate, float intervalLength)
{
	return sampleRate * intervalLength;
}

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: addBuffer
--
-- DATE: April 4, 2015
--
-- REVISIONS:
--
-- DESIGNER: Calvin Rempel
--
-- PROGRAMMER: Calvin Rempel
--
-- INTERFACE: addBuffer()
--
-- RETURNS: void
--
-- NOTES: This function adds a new buffer to the WaveIn structure, allowing for more data to be
-- recorded. It results in memory being allocated with malloc which should be freed in the
-- waveInProc().
-------------------------------------------------------------------------------------------------*/
void MicReader::addBuffer()
{
	WAVEHDR *hdr = (WAVEHDR*) malloc(sizeof(WAVEHDR));

	hdr->lpData = (LPSTR)(new int[buffLen]);
	hdr->dwBufferLength = this->buffLen;
	hdr->dwBytesRecorded = 0;
	hdr->dwUser = 0l;
	hdr->dwFlags = 0l;
	hdr->dwLoops = 0l;

	waveInPrepareHeader(mic, hdr, sizeof(WAVEHDR));
	waveInAddBuffer(mic, hdr, sizeof(WAVEHDR));
	wavHeaders.push(hdr);
}

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: readIn
--
-- DATE: April 4, 2015
--
-- REVISIONS:
--
-- DESIGNER: Calvin Rempel
--
-- PROGRAMMER: Calvin Rempel
--
-- INTERFACE: readIn()
--
-- RETURNS: void
--
-- NOTES: This function attempts to open the microphone and begin reading from it into the
-- readers buffers.
-------------------------------------------------------------------------------------------------*/
void MicReader::readIn()
{
	// Attempt to open the Microphone for reading.
	result = waveInOpen(&mic, WAVE_MAPPER, &format, (DWORD_PTR)MicReader::waveInProc, 0L, WAVE_FORMAT_DIRECT | CALLBACK_FUNCTION);

	if (result)
	{
		#ifdef DEBUG
		MessageBox(NULL, L"Error opening microphone.", L"Error", MB_ICONERROR);
		#endif
		return;
	}
	else
	{
		// Add an initial buffers to start with
		for(int i = 0; i < 10; ++i)
		{
			addBuffer();
		}

		if (result)
		{
			#ifdef DEBUG
			MessageBox(NULL, L"Error preparing WAV Buffer.", L"Error", MB_ICONERROR);
			#endif
			return;
		}

		// Start reading from the microphone into it's buffers.
		result = waveInStart(mic);

		if (result)
		{
			#ifdef DEBUG
			MessageBox(NULL, L"Error reading data from microphone.", L"Error", MB_ICONERROR);
			#endif
			return;
		}
	}
}

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: waveInProc
--
-- DATE: April 4, 2015
--
-- REVISIONS:
--
-- DESIGNER: Calvin Rempel
--
-- PROGRAMMER: Calvin Rempel
--
-- INTERFACE: waveInProc(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
--		HWAVEIN hwi			 : The Handle of the microphone that generated the callback call
--		UINT uMsg			 : The event that triggered the call (WIM_OPEN, WIM_CLOSE, WIM_DATA)
--		DWORD_PTR dwInstance : An instance of the callback
--		DWORD_PTR dwParam1   : A generic parameter for the callback
--		DWORD_PTR dwParam2   : A second generic parameter for the callback
--
-- RETURNS: void
--
-- NOTES: This function is a Windows callback that is called whenever a microphone is opened,
-- closed, or has data available for reading. If data is available, this function copies that
-- data into the MicReaders' MessageQueue, adds a new buffer to allow for continuous reading,
-- frees the used buffer.
--
-- If the event is WIM_CLOSE, this function clears all memory required by the buffers and sends
-- a WM_MIC_STOPPED_READING message through the Window Message Pump to it's owner.
-------------------------------------------------------------------------------------------------*/
void CALLBACK MicReader::waveInProc(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
	// If data is available from the microphone, copy into a message queue
	if (uMsg == WIM_DATA)
	{
		MicReader *micReader = MicReader::getInstance();
		WAVEHDR *completed = (WAVEHDR*) dwParam1;

		// Add the recorded data to the output message queue
		micReader->mqueue->enqueue(MIC_INPUT_MQUEUE_TYPE, completed->lpData);

		// Add another buffer to read into
		if (micReader->recording)
			micReader->addBuffer();

		// Remove the used up buffer
		if (micReader->wavHeaders.size() > 0)
		{
			delete micReader->wavHeaders.front()->lpData;
			free(micReader->wavHeaders.front());
			micReader->wavHeaders.pop();
		}
	}
	// If recording has stopped, clean up buffers and post message to WM pump
	else if (uMsg == WIM_CLOSE)
	{
		MicReader *micReader = MicReader::getInstance();
		micReader->recording = false;

		// Free all allocated buffers
		while (micReader->wavHeaders.size() > 0)
		{
			delete micReader->wavHeaders.front()->lpData;
			free(micReader->wavHeaders.front());
			micReader->wavHeaders.pop();
		}

		// Send a WM message to indicate the recording has stopped
		SendMessage(micReader->owner, WM_MIC_STOPPED_READING, 0, 0);
	}
}
