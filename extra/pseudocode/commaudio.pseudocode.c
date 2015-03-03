////////////
// client //
////////////

Client::startClient()
{
    TransmitThreadController.start();
    OutputThreadController.start();
    VoiceBuffererThreadController.start();
    MusicBuffererThreadController.start();
    ControlThreadController.start(); // will also start the receive thread
}
Client::stopClient()
{
    ControlThreadController.stop(); // will also stop the receive thread
    MusicBuffererThreadController.stop();
    VoiceBuffererThreadController.stop();
    OutputThreadController.stop();
    TransmitThreadController.stop();
}

Client::pauseStream()
{
    MusicReaderThreadController.stop();
}
Client::resumeStream()
{
    MusicReaderThreadController.play();
}
Client::stopStream()
{
    MusicReaderThreadController.stop();
    MusicReaderThreadController.seek(0);
}
Client::seekStream(percent)
{
    MusicReaderThreadController.seek(percent);
}

Client::startTransmission()
{
    RecorderThreadController.start();
}
Client::stopTransmission()
{
    RecorderThreadController.stop();
}
Client::setTransmissionGroup(groupId)
{
    TransmitThreadController.setGroup(groupId);
}

Client::startDownload(songId)
{
    ControlThreadController.requestDownload(songId);
}
Client::stopDownload(songId)
{
    ControlThreadController.cancelDownload(songId);
}
Client::changeStream(songId)
{
    ControlThreadController.changeStream(songId);
}

SERVER
======

Server::startServer()
Server::stopServer()
Server::setServerPort(port)
Server::changeStream()
Server::onConnect()
{
    SessionThreadControllerFactory.make().start()
}

Server::onUpload(songID)
Server::onDisconnect()
{
    for all UTCBs in the UTCBSet
    {
        UTCB.stop = true
    }

    wait for all of the UTCB.stopped events to become set
}

Server::STCB
{
    Event stop
    Event stopped
    MessageQueue msgQueue
    UTCBSet
}

Server::UTCB
{
    bool stop
    Event stopped
    int songID
}

Server::UploadThread
{
    reset UTCB.stopped
    figure out where the file is from the UTCB.songID
    open the file
    while(!UTCB.stop)
    {
        read file
        enqueue song packet to STCB.msgQueue
    }
    close the file
    set UTCB.stopped
}
