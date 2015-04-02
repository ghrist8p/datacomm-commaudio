
// #include <windows.h>
// #include <mmsystem.h>
// #include <stdio.h>
// #include <iostream>

// //#define TEST

// #define SAMPLE_RATE 44100
// #define BITS_PER_SAMPLE 16
// #define CHANNELS 2
// #define BITS_PER_BYTE 8
// #define BUFSIZE (SAMPLE_RATE*BITS_PER_SAMPLE/BITS_PER_BYTE*CHANNELS)
// #define MILLISEC_PER_SEC 1000

// using namespace std;

// #pragma warning(disable:4996)
// #pragma comment(lib,"winmm.lib")

// struct WavSong
// {
//     char* data;
//     unsigned long size;
// };

// //add wimm.lib to the libraries list

// HWAVEOUT hWaveOut = 0; // device handle

// void startWave()
// {
//     WAVEFORMATEX wfx;  // settings stuff

//     wfx.nSamplesPerSec = SAMPLE_RATE;
//     wfx.wBitsPerSample = BITS_PER_SAMPLE;
//     wfx.nChannels      = CHANNELS;

//     wfx.cbSize = 0; /* size of _extra_ info */
//     wfx.wFormatTag = WAVE_FORMAT_PCM;
//     wfx.nBlockAlign = (wfx.wBitsPerSample >> 3) * wfx.nChannels;
//     wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;

//     if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR)
//     {
//         fprintf(stderr, "unable to open WAVE_MAPPER device\n");
//         return;
//     }
// }
// void playWave(WAVEHDR* audioPacket, char* data, int length)
// {
//     memset(audioPacket,0,sizeof(WAVEHDR));
//     audioPacket->lpData = (char*)data;
//     audioPacket->dwBufferLength = length;

//     //prepare the header.
//     waveOutPrepareHeader(hWaveOut, audioPacket, sizeof(WAVEHDR));
//     waveOutWrite(hWaveOut, audioPacket, sizeof(WAVEHDR));
//     waveOutUnprepareHeader(hWaveOut, audioPacket, sizeof(WAVEHDR));
// }

// #ifdef TEST
// int main(void)
// {
//     startWave();
//     FILE* fp = fopen("C:\\Users\\Eric\\Downloads\\Egoist_-_Extra_Terrestrial_Biological_Entities.wav","rb");
//     WAVEHDR audioPacket1;
//     WAVEHDR audioPacket2;
//     WAVEHDR audioPacket3;
//     WAVEHDR audioPacket4;
//     WAVEHDR audioPacket5;
//     WAVEHDR audioPacket6;
//     WAVEHDR audioPacket7;
//     WAVEHDR audioPacket8;
//     char* buffer1 = (char*) malloc(BUFSIZE);
//     char* buffer2 = (char*) malloc(BUFSIZE);
//     char* buffer3 = (char*) malloc(BUFSIZE);
//     char* buffer4 = (char*) malloc(BUFSIZE);
//     char* buffer5 = (char*) malloc(BUFSIZE);
//     char* buffer6 = (char*) malloc(BUFSIZE);
//     char* buffer7 = (char*) malloc(BUFSIZE);
//     char* buffer8 = (char*) malloc(BUFSIZE);
//     fread(buffer1,1,BUFSIZE,fp);
//     fread(buffer2,1,BUFSIZE,fp);
//     fread(buffer3,1,BUFSIZE,fp);
//     fread(buffer4,1,BUFSIZE,fp);
//     fread(buffer5,1,BUFSIZE,fp);
//     fread(buffer6,1,BUFSIZE,fp);
//     fread(buffer7,1,BUFSIZE,fp);
//     fread(buffer8,1,BUFSIZE,fp);
//     playWave(&audioPacket1,buffer1,BUFSIZE);
//     playWave(&audioPacket2,buffer2,BUFSIZE);
//     playWave(&audioPacket3,buffer3,BUFSIZE);
//     playWave(&audioPacket4,buffer4,BUFSIZE);
//     playWave(&audioPacket5,buffer5,BUFSIZE);
//     playWave(&audioPacket6,buffer6,BUFSIZE);
//     playWave(&audioPacket7,buffer7,BUFSIZE);
//     playWave(&audioPacket8,buffer8,BUFSIZE);
//     while(!(audioPacket1.dwFlags&WHDR_DONE));
//     free(buffer1);
//     printf("done\n");
//     while(!(audioPacket2.dwFlags&WHDR_DONE));
//     free(buffer2);
//     printf("done\n");
//     while(!(audioPacket3.dwFlags&WHDR_DONE));
//     free(buffer3);
//     printf("done\n");
//     while(!(audioPacket4.dwFlags&WHDR_DONE));
//     free(buffer4);
//     printf("done\n");
//     while(!(audioPacket5.dwFlags&WHDR_DONE));
//     free(buffer5);
//     printf("done\n");
//     while(!(audioPacket6.dwFlags&WHDR_DONE));
//     free(buffer6);
//     printf("done\n");
//     while(!(audioPacket7.dwFlags&WHDR_DONE));
//     free(buffer7);
//     printf("done\n");
//     while(!(audioPacket8.dwFlags&WHDR_DONE));
//     free(buffer8);
//     printf("done\n");

//     fclose(fp);

//     getchar();
//     return 0;
// }
// #endif
