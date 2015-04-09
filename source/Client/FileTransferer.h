/*-----------------------------------------------------------------------------
-- SOURCE FILE: FileTransferer.h - This file provides functionality for
-- transferring files between multiple computers.
--
-- PUBLIC FUNCTIONS:
-- void sendFile(char *filename, TCPSocket *socket);
-- void recvFile(char *data);
-- void cancelTransfer(char *filename, TCPSocket *socket);
--
-- DATE:
--
-- REVISIONS:
--
-- DESIGNER: Calvin Rempel
--
-- PROGRAMMER: Calvin Rempel
--
-- NOTES:
-- This class will SEND transfer data using TCPSockets, but must be fed
-- incoming data into recvFile.
-----------------------------------------------------------------------------*/

#ifndef _FILE_TRANSFERER_H_
#define _FILE_TRANSFERER_H_

#include "../common.h"
#include <map>

#define FILENAME_PACKET_LENGTH 128
#define FILE_PACKET_SIZE 256

class FileTransferer;

/*
	A Callback type called when a download has finished transferring.
*/
typedef void(*OnDownloadComplete)(char *filename, bool success);


/*
	Information used internally to utilize threads.
*/
struct FileTransferInfo
{
	FileTransferer *pThis;
	TCPSocket *socket;
	FileTransferData *data;
};

struct SongName;

/*-----------------------------------------------------------------------------
-- CLASS: FileTransferer
--
-- DESCRIPTION: This class provides functionality for sending/receiving files
-- across multiple computers. It will keep track of multiple downloads/uploads
-- and send a callback when a download has finished.
--
-- When a Download has finished transferring (successfully or not), the
-- OnDownloadComplete callback is called indicating the status.
-----------------------------------------------------------------------------*/
class FileTransferer
{
	public:
		/* CONSTRUCTORS/DESTRUCTORS */
		FileTransferer(OnDownloadComplete downloadComplete);
		~FileTransferer(){};

		/* PUBLIC MEMBER METHODS */
		void sendFile(SongName *song, TCPSocket *socket);
		void recvFile(char *data);
		void cancelTransfer(int songId, TCPSocket *socket);

	private:
		/* PRIVATE STATIC MEMBER METHODS */
		static DWORD WINAPI TransferThread(LPVOID transferInfo);

		/* PRIVATE MEMBER DATA */
		std::map<int, std::map<TCPSocket*, FILE*>> filesOut;
		std::map<int, FILE*> filesIn;
		std::map<int, std::map<TCPSocket*, bool>> transferring;
		OnDownloadComplete onDownloadComplete;
};

#endif
