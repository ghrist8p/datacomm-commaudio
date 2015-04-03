#ifndef _PLAY_WAVE_H_
#define _PLAY_WAVE_H_

#include "../common.h"
#include <stdio.h>

class MessageQueue;

/**
 * used to decode & play audio out speakers from raw PCM data.
 */
class PlayWave
{
public:
	PlayWave(int capacity, MessageQueue* msgq);
	~PlayWave();
	int startPlaying(int samplesPerSecond, int bitsPerSample, int numChannels);
	int stopPlaying();

private:
	int openDevice(int samplesPerSecond, int bitsPerSample, int numChannels);
	int closeDevice();
	void enqueueAudioData(char* data, int length);
	static DWORD WINAPI playRoutine(void* params);
	void handleMsgqMsg();
	static void freeAudioPacketOnceUsed(PlayWave* dis, WAVEHDR* audioPacket);
	static DWORD WINAPI freeAudioPacketOnceUsedRoutine(void* params);

	/**
	 * pointer to the last enqueued audio packet if it exists; 0 otherwise.
	 *
	 * a pointer to the last packet is maintained, so when another packet is
	 *   enqueued for playing, we can set the last packet's user pointer to the
	 *   new packet, creating a linked list of audio packets.
	 *
	 * the idea, is to have a thread follow the linked list of enqueued audio
	 *   packets, and deallocate them as they are played.
	 */
	WAVEHDR* lastAudioPacket;

	/**
	 * mutex used to protect the _lastAudioPacket linked list.
	 */
	HANDLE lastAudioPacketAccess;

	/**
	 * handle to a semaphore that is 0 then the queue is empty, positive
	 *   otherwise.
	 */
	HANDLE canDequeue;

	/**
	 * handle to a semaphore that is 100 then the queue is empty, positive
	 *   otherwise.
	 */
	HANDLE canEnqueue;

	/**
	 * handle to thread used to dequeue audio from the message queue, and add
	 *   them to Window's play buffer to be played.
	 */
	HANDLE playThread;

	/**
	 * handle to thread used to dequeue audio from the message queue, and add
	 *   them to Window's play buffer to be played.
	 */
	HANDLE playThreadStopEv;

	/**
	 * pointer to the message queue used by this instance to read audio data
	 *   from.
	 */
	MessageQueue* msgq;

	/**
	 * settings structure used to describe the audio device used to play things
	 */
	WAVEFORMATEX wfx;

	/**
	 * handle to the system's primary speakers; 0 if not valid. non-zero
	 *   otherwise.
	 */
	HWAVEOUT speakers;
};

#endif
