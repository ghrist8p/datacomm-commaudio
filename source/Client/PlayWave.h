#ifndef _PLAY_WAVE_H_
#define _PLAY_WAVE_H_

#include <windows.h>
#include <stdio.h>

class PlayWave
{
public:
	PlayWave();
	void startWave();
	void playWave(char *data);

private:
	HWAVEOUT hWaveOut;
};

#endif