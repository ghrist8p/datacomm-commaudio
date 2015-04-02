
#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <iostream>

using namespace std;

#pragma warning(disable:4996)
#pragma comment(lib,"Winmm.lib")

struct WavSong
{
	char* data;
	unsigned long size;
};

//add wimm.lib to the libraries list

HWAVEOUT hWaveOut = 0; // device handle

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
WAVEHDR PiecetoPlay;
void playWave(char* data, int length)
{
	PiecetoPlay.lpData = (char*)data;
    PiecetoPlay.dwBufferLength = length;
	//implement the length and all encessary data.

	//prepare the header.
	waveOutPrepareHeader(hWaveOut, &PiecetoPlay, sizeof(PiecetoPlay));
	waveOutWrite(hWaveOut, &PiecetoPlay, sizeof(PiecetoPlay));
    waveOutUnprepareHeader(hWaveOut, &PiecetoPlay, sizeof(PiecetoPlay));
}

#define SAMPLE_RATE 44100
#define BITS_PER_SAMPLE 16
#define CHANNELS 2
#define BITS_PER_BYTE 8
#define BUFSIZE (SAMPLE_RATE*BITS_PER_SAMPLE/BITS_PER_BYTE*CHANNELS)
#define MILLISEC_PER_SEC 1000

int main(void)
{
    memset(&PiecetoPlay,0,sizeof(PiecetoPlay));
    startWave();
    FILE* fp = fopen("C:\\Users\\Eric\\Downloads\\Egoist_-_Extra_Terrestrial_Biological_Entities.wav","rb");
    char* buffer1 = (char*) malloc(BUFSIZE);
    char* buffer2 = (char*) malloc(BUFSIZE);
    int data_read = 0;
    int bytesRead;
    fread(buffer1,1,BUFSIZE,fp);
    fread(buffer2,1,BUFSIZE,fp);
    playWave(buffer1,BUFSIZE);
    printf("im done looping!\n");
    Sleep(1000);
    printf("im done looping!\n");
    playWave(buffer2,BUFSIZE);
    //while((bytesRead = fread(buffer,1,BUFSIZE,fp)) > 0)
    //{
    //    printf("bytesRead: %d/%d - %d\n",bytesRead,BUFSIZE,bytesRead/BUFSIZE);
    //    Sleep(10);
    //    //Sleep(bytesRead/BUFSIZE*1000+10);
    //    printf("im done sleeping!\n");
    //}
    //printf("im done looping!\n");
	//while (fread(buffer, 1, 1000, fp) > 0)
	//{
       // playWave(buffer,999999);
	//}

        //while ((data_read = fre/*ad(buffer, 1, speed, fp)) > 0)
		/*{*/
					//we should add a flag to stop this if another song is being sent.
                    //printf("data_read: %d\n",data_read);
            
        /*}*/


            
    

 //   int speed = 1000;
 //   WavSong* ret = new WavSong();
	//FILE* fp = fopen("C:/Users/Eric/Documents/Visual Studio 2012/Projects/commaudio/Debug/sound.wav","r");
 //   if(fp)
 //   {
 //       printf("fp is open");
 //   }
 //   else
 //   {
 //       perror("fp is closed");
 //   }

 //   if (fp) {

	//	char id[5];
	//	unsigned long size;
	//	short format_tag, channels, block_align, bits_per_sample;
	//	unsigned long format_length, sample_rate, avg_bytes_sec, data_size;
	//	int data_read = 0;

	//	fread(id, sizeof(char), 4, fp);
	//	id[4] = '\0';

	//	if (!strcmp(id, "RIFF")) {
	//		fread(&size, sizeof(unsigned long), 1, fp);
	//		fread(id, sizeof(char), 4, fp);
	//		id[4] = '\0';

	//		if (!strcmp(id, "WAVE")) {
	//			//get wave headers
	//			fread(id, sizeof(char), 4, fp);
	//			fread(&format_length, sizeof(unsigned long), 1, fp);
	//			fread(&format_tag, sizeof(short), 1, fp);
	//			fread(&channels, sizeof(short), 1, fp);
	//			fread(&sample_rate, sizeof(unsigned long), 1, fp);
	//			fread(&avg_bytes_sec, sizeof(unsigned long), 1, fp);
	//			fread(&block_align, sizeof(short), 1, fp);
	//			fread(&bits_per_sample, sizeof(short), 1, fp);
	//			fread(id, sizeof(char), 4, fp);
	//			fread(&data_size, sizeof(unsigned long), 1, fp);

	//			ret->data = (char*)malloc(speed);

	//			//read chunks of data from the file based on the speed selected and send it
	//			while ((data_read = fread(ret->data, 1, speed, fp)) > 0)
	//			{
	//				//we should add a flag to stop this if another song is being sent.
 //                   printf("data_read: %d\n",data_read);
	//				playWave(ret->data,data_read);
 //                   
	//			}
 //               printf("fuck this!\n");
	//		}
	//		else {
	//			cout << "Error: RIFF file but not a wave file\n";
	//		}
	//	}
	//	else {
	//		cout << "Error: not a RIFF file\n";
	//	}
	//}

	fclose(fp);

    getchar();
    return 0;
}
