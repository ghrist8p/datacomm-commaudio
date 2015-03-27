#include "MessageQueue.h"

class ClientControlThread
{
public:
    ClientControlThread();
    ~ClientControlThread();
    void requestPacketRetransmission(int index);
    void requestDownload(char* file);
    void cancelDownload(char* file);
    void changeStream(char* file);
    void onDownloadPacket(int index, void* data, int len);
    void onRetransmissionPacket(int index, void* data, int len);
    void onChangeStream(char* file);
private:
    static DWORD WINAPI _threadRoutine(void* params);
    MessageQueue _msgq;
};
