#ifndef _VOICE_BUFFERER_H_
#define _VOICE_BUFFERER_H_

#include "../common.h"

class UDPSocket;
class JitterBuffer;

class VoiceBufferer
{
public:
    VoiceBufferer(
        MessageQueue* speakerQueue,
        JitterBuffer* voiceJitterBuffer);
    ~VoiceBufferer();
    void start();
    void stop();
private:
    static DWORD WINAPI _threadRoutine(void* params);
    MessageQueue* speakerQueue;
    JitterBuffer* voiceJitterBuffer;
    HANDLE thread;
    HANDLE threadStopEv;
};

#endif
