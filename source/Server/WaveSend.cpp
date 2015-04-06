/*-------------------------------------------------------------------------------------------------
-- SOURCE FILE: WaveSend.cpp
--
-- PROGRAM: CommAudio
--
-- FUNCTIONS:
-- void sendWaveFile(char* fname, WavSong *ret, int speed)
--
--
-- DATE: March 28, 2015
--
-- REVISIONS:
--
-- DESIGNER: Manuel Gonzales
--
-- PROGRAMMER: Manuel Gonzales
--
-- NOTES:
-- Really early version to test music streaming of wave files.
-------------------------------------------------------------------------------------------------*/




//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <iostream>
//using namespace std;
//
//
//struct WavSong
//{
//	short* data;
//	unsigned long size;
//};
//
//
//void sendWaveFile(char* fname, WavSong *ret, int speed)
//{
//
//	FILE* fp = fopen(fname, "rb");
//	if (fp) {
//
//		char id[5];
//		unsigned long size;
//		short format_tag, channels, block_align, bits_per_sample;
//		unsigned long format_length, sample_rate, avg_bytes_sec, data_size;
//
//		fread(id, sizeof(char), 4, fp);
//		id[4] = '\0';
//
//		if (!strcmp(id, "RIFF")) {
//			fread(&size, sizeof(unsigned long), 1, fp);
//			fread(id, sizeof(char), 4, fp);
//			id[4] = '\0';
//
//			if (!strcmp(id, "WAVE")) {
//				fread(id, sizeof(char), 4, fp);
//				fread(&format_length, sizeof(unsigned long), 1, fp);
//				fread(&format_tag, sizeof(short), 1, fp);
//				fread(&channels, sizeof(short), 1, fp);
//				fread(&sample_rate, sizeof(unsigned long), 1, fp);
//				fread(&avg_bytes_sec, sizeof(unsigned long), 1, fp);
//				fread(&block_align, sizeof(short), 1, fp);
//				fread(&bits_per_sample, sizeof(short), 1, fp);
//				fread(id, sizeof(char), 4, fp);
//				fread(&data_size, sizeof(unsigned long), 1, fp);
//
//				ret->size = data_size / sizeof(short);
//				ret->data = (short*)malloc(data_size);
//
//				while (fread(ret->data, sizeof(short), speed / ret->size, fp) > 0)
//				{
//
//					//SEND TO GROUP? 
//					//not sure about sending just plain data or COMPRESSING IT.
//				}
//			}
//			else {
//				cout << "Error: RIFF file but not a wave file\n";
//			}
//		}
//		else {
//			cout << "Error: not a RIFF file\n";
//		}
//	}
//	fclose(fp);
//}