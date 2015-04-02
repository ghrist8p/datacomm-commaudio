#include "MicReader.h"
#include <iostream>

#define SAMPLE_RATE 44100
#define BITS_PER_SAMPLE 16
#define CHANNELS 2
#define BITS_PER_BYTE 8
#define BUFSIZE (SAMPLE_RATE*BITS_PER_SAMPLE/BITS_PER_BYTE*CHANNELS)

MicReader::MicReader(int numSeconds)
{
	recordLength = numSeconds;
	sampleRate = SAMPLE_RATE;
	buffLen = BUFSIZE * numSeconds;
	waveIn = new short[buffLen];

	result = 0;
	format.wFormatTag = WAVE_FORMAT_PCM;
	format.wBitsPerSample = BITS_PER_SAMPLE;
	format.nChannels = CHANNELS;
	format.nSamplesPerSec = sampleRate;
	format.nAvgBytesPerSec = format.nSamplesPerSec * format.nChannels * format.wBitsPerSample / 8;
	format.nBlockAlign = format.nChannels * format.wBitsPerSample / BITS_PER_BYTE;
	format.cbSize = 0;
}


MicReader::~MicReader()
{
	delete waveIn;
}

void MicReader::startReading()
{
	result = waveInOpen(&mic, WAVE_MAPPER, &format, 0L, 0L, WAVE_FORMAT_DIRECT);

	if (result)
	{
		std::cout << "Error opening Microphone" << std::endl;
		return;
	}

	wavHeader.lpData = (LPSTR)waveIn;
	wavHeader.dwBufferLength = buffLen * 2;
	wavHeader.dwBytesRecorded = 0;
	wavHeader.dwUser = 0l;
	wavHeader.dwFlags = 0l;
	wavHeader.dwLoops = 0l;

	waveInPrepareHeader(mic, &wavHeader, sizeof(WAVEHDR));
	result = waveInAddBuffer(mic, &wavHeader, sizeof(WAVEHDR));

	if (result)
	{
		std::cout << "Error preparing wave in buffer" << std::endl;
		return;
	}

	result = waveInStart(mic);

	if (result)
	{
		std::cout << "Error reading from microphone" << std::endl;
		return;
	}

	do {} while (waveInUnprepareHeader(mic, &wavHeader, sizeof(WAVEHDR)) == WAVERR_STILLPLAYING);
	waveInClose(mic);

	MessageBox(NULL, L"DONE RECORDING", L"APPARENTLY", MB_ICONASTERISK);
}

void MicReader::stopReading()
{

}

char *MicReader::getRecordedData()
{
	return (char*) waveIn;
}
