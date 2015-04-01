#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>

//add wimm.lib to the libraries list

HWAVEOUT hWaveOut; // device handle

void startWave()
{	
	WAVEFORMATEX wfx;  // settings stuff
	

	wfx.nSamplesPerSec = 44100; /* sample rate */
	wfx.wBitsPerSample = 16; /* sample size */
	wfx.nChannels = 2; /* channels*/
	

	wfx.cbSize = 0; /* size of _extra_ info */
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nBlockAlign = (wfx.wBitsPerSample >> 3) * wfx.nChannels;
	wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;

	if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0,	CALLBACK_NULL) != MMSYSERR_NOERROR)
	{
		fprintf(stderr, "unable to open WAVE_MAPPER device\n");
		return;
	}

}

void playWave(char* data)
{
	WAVEHDR PiecetoPlay;
	PiecetoPlay.lpData = data;
	//implement the length and all encessary data.


	//prepare the header.
	waveOutPrepareHeader(hWaveOut, (LPWAVEHDR)&PiecetoPlay, sizeof(PiecetoPlay));

	waveOutWrite(hWaveOut, (LPWAVEHDR)&PiecetoPlay, sizeof(PiecetoPlay));
}