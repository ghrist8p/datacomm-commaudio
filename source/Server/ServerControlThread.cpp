
/////////////////////////////////////////
// static function forward declaration //
/////////////////////////////////////////
static int startRoutine(HANDLE* thread, HANDLE stopEvent,
    LPTHREAD_START_ROUTINE routine, void* params);
static int stopRoutine(HANDLE* thread, HANDLE stopEvent);

ServerControlThread* getInstance()
{
    // acquire synchronization objects
    WaitForSingleObject(access,INFINITE);

    // retrieve singleton and make if needed
    if(_instance == 0)
    {
        _instance = new ServerControlThread();
    }

    // release synchronization objects
    ReleaseMutex(access);

    return _instance;
}

ServerControlThread::ServerControlThread()
    :_msgq(MSGQ_CAPACITY,MSGQ_ELEM_SIZE)
    ,_sockMsgq(SOCK_MSGQ_CAPACITY,SOCK_MSGQ_ELEM_SIZE)
{
    // initialize instance variables
    _threadStopEv = CreateEvent(NULL,TRUE,FALSE,NULL);
    _thread       = INVALID_HANDLE_VALUE;
}

ServerControlThread::~ServerControlThread()
{

}

void ServerControlThread::start(short port)
{
    this->port = port;
    _startRoutine(&_thread,_threadStopEv,_threadRoutine,this);
}

void ServerControlThread::stop()
{
    _stopRoutine(&_thread,_threadStopEv);
}

DWORD WINAPI ServerControlThread::_threadRoutine(void* params)
{

}

/////////////////////////////////////
// static function implementations //
/////////////////////////////////////

int startRoutine(HANDLE* thread, HANDLE stopEvent,
    LPTHREAD_START_ROUTINE routine, void* params)
{
    // return immediately if the routine is already running
    if(*thread != INVALID_HANDLE_VALUE)
    {
        return 1;
    }

    // reset the stop event
    ResetEvent(stopEvent);

    // start the thread & return
    DWORD useless;
    *thread = CreateThread(0,0,routine,params,0,&useless);
    return (*thread == INVALID_HANDLE_VALUE);
}

int stopRoutine(HANDLE* thread, HANDLE stopEvent)
{
    // return immediately if the routine is already stopped
    if(*thread == INVALID_HANDLE_VALUE)
    {
        return 1;
    }

    // set the stop event to stop the thread
    SetEvent(stopEvent);
    WaitForSingleObject(*thread,INFINITE);

    // invalidate thread handle, so we know it's terminated
    *thread = INVALID_HANDLE_VALUE;
    return 0;
}
