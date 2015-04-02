#include "PlayWave.h"

#include <windows.h>
#include <stdio.h>

#define SAMPLE_RATE 44100
#define BITS_PER_SAMPLE 16
#define CHANNELS 2
#define BITS_PER_BYTE 8
#define BUFSIZE (SAMPLE_RATE*BITS_PER_SAMPLE/BITS_PER_BYTE*CHANNELS)
#define MILLISEC_PER_SEC 1000

using namespace std;

#pragma warning(disable:4996)
#pragma comment(lib,"winmm.lib")

// static function forward declarations
static void deleteAudioPacketOnceUsed(WAVEHDR* audioPacket);

PlayWave::PlayWave()
{
	WAVEFORMATEX wfx;  // settings stuff

	wfx.nSamplesPerSec = SAMPLE_RATE;
	wfx.wBitsPerSample = BITS_PER_SAMPLE;
	wfx.nChannels      = CHANNELS;

	wfx.cbSize          = 0;
	wfx.wFormatTag      = WAVE_FORMAT_PCM;
	wfx.nBlockAlign     = (wfx.wBitsPerSample >> 3) * wfx.nChannels;
	wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;

	if(waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR)
	{
		fprintf(stderr, "unable to open WAVE_MAPPER device\n");
		return;
	}
}

void PlayWave::playWave(char* data, int length)
{
	// allocate, and set header data
	WAVEHDR* audioPacket = (WAVEHDR*) malloc(sizeof(*audioPacket));
	memset(audioPacket,0,sizeof(WAVEHDR));
	audioPacket->lpData         = data;
	audioPacket->dwBufferLength = length;

	// prepare the header.
	waveOutPrepareHeader(hWaveOut, audioPacket, sizeof(WAVEHDR));
	waveOutWrite(hWaveOut, audioPacket, sizeof(WAVEHDR));
	waveOutUnprepareHeader(hWaveOut, audioPacket, sizeof(WAVEHDR));

	// delete the header once it was used
	deleteAudioPacketOnceUsed(audioPacket);
}

void deleteAudioPacketOnceUsed(WAVEHDR* audioPacket)
{

}
