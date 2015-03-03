
/////////////////////
// data structures //
/////////////////////

// MessageQueue
    void enqueue(Message msg);
    void dequeue(Message msg);
    Event canDequeue;
// Buffer
    void blockingEnqueue(void* data,int len);
    void blockingDequeue(void* data,int len);
// JitterBuffer
    void insert(long index,void* data,int len);
    void dequeue(void* data,int len);
    Event canDequeueChanged; // triggered when canDequeue increases
    int canDequeue; // indicates how much data can be dequeued
    // can dequeue when data has been enqueued for timeout time, or buffer hits high mark

///////////////////////////////
// client thread controllers //
///////////////////////////////

// ReceiveThreadController
    start();
    stop();
// TransmitThreadController
    start();
    stop();
    setGroup();
// MusicBuffererThreadController
    start();
    stop();
    insertMusicData();
// VoiceBuffererThreadController
    start();
    stop();
// ControlThreadController
    requestMusicData();
    requestDownload();
    appendDownloadData();
    cancelDownload();
    changeStream();
// RecorderThreadController
    start();
    stop();
// MusicReaderThreadController
    play();
    stop();
    seek();
// OutputThreadController
    start();
    stop();

///////////////////////////////
// server thread controllers //
///////////////////////////////

// SessionManagerThreadController
    start();
    stop();
    waitUntilStopped();
// SessionThreadController
    start();
    stop();
    waitUntilStarted();
    waitUntilStopped();
// StreamThreadController
    start();
    stop();
    waitUntilStarted();
    waitUntilStopped();
// UploadThreadController
    start();
    stop();
