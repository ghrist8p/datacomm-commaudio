#include "MicReader.h"
#include <iostream>

#include "../Buffer/MessageQueue.h"

MicReader::MicReader(int sampleRate, int intervalLength, MessageQueue *queue, HWND owner)
{
	this->mqueue = queue;
	this->recordLength = intervalLength;
	this->buffLen = calculateBufferSize(sampleRate, intervalLength);
	this->waveIn = new short[buffLen];
	this->recording = false;
	this->owner = owner;

	result = 0;
	format.wFormatTag = WAVE_FORMAT_PCM;
	format.wBitsPerSample = BITS_PER_SAMPLE;
	format.nChannels = 1;
	format.nSamplesPerSec = sampleRate;
	format.nAvgBytesPerSec = format.nSamplesPerSec * format.nChannels * format.wBitsPerSample / BITS_PER_SAMPLE;
	format.nBlockAlign = format.nChannels * format.wBitsPerSample / BITS_PER_SAMPLE;
	format.cbSize = 0;
}


MicReader::~MicReader()
{
	delete waveIn;
}

void MicReader::startReading()
{
	recording = true;
	DWORD threadId;
	CreateThread(NULL, NULL, MicReader::RecordThread, (void*) this, NULL, &threadId);
}

void MicReader::stopReading()
{
	recording = false;
}

size_t MicReader::calculateBufferSize(int sampleRate, int intervalLength)
{
	return sampleRate * intervalLength;
}

DWORD WINAPI MicReader::RecordThread(LPVOID lpParam)
{
	MicReader *pThis = (MicReader*) lpParam;

	pThis->result = waveInOpen(&pThis->mic, WAVE_MAPPER, &pThis->format, 0L, 0L, WAVE_FORMAT_DIRECT);

	if (pThis->result)
	{
		std::cout << "Error opening Microphone" << std::endl;
		return 1;
	}
	else
	{
		while (pThis->recording)
		{
			pThis->wavHeader.lpData = (LPSTR)pThis->waveIn;
			pThis->wavHeader.dwBufferLength = pThis->buffLen * 2;
			pThis->wavHeader.dwBytesRecorded = 0;
			pThis->wavHeader.dwUser = 0l;
			pThis->wavHeader.dwFlags = 0l;
			pThis->wavHeader.dwLoops = 0l;

			waveInPrepareHeader(pThis->mic, &pThis->wavHeader, sizeof(WAVEHDR));
			pThis->result = waveInAddBuffer(pThis->mic, &pThis->wavHeader, sizeof(WAVEHDR));

			if (pThis->result)
			{
				std::cout << "Error preparing wave in buffer" << std::endl;
				return 1;
			}

			pThis->result = waveInStart(pThis->mic);

			if (pThis->result)
			{
				std::cout << "Error reading from microphone" << std::endl;
				return 1;
			}

			do {} while (waveInUnprepareHeader(pThis->mic, &pThis->wavHeader, sizeof(WAVEHDR)) == WAVERR_STILLPLAYING);
			pThis->mqueue->enqueue(MIC_INPUT_MQUEUE_TYPE, pThis->waveIn);
			memset(pThis->waveIn, 0, pThis->buffLen);
		}
	}

	waveInClose(pThis->mic);
	SendMessage(pThis->owner, WM_MIC_STOPPED_READING, 0, 0);
}