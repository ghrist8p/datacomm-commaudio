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

#include "FileTransferer.h"

#include <fstream>
#include <string>
#include <errno.h>

#include "../protocol.h"

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: FileTransferer
--
-- DATE:
--
-- REVISIONS:
--
-- DESIGNER: Calvin Rempel
--
-- PROGRAMMER: Calvin Rempel
--
-- INTERFACE: FileTransferer(OnDownloadComplete downloadComplete)
--		OnDownloadComplete downloadComplete	 : the callback to call when a download stops.
--
-- NOTES: Create a new FileTransferer
-------------------------------------------------------------------------------------------------*/
FileTransferer::FileTransferer(OnDownloadComplete downloadComplete)
	: onDownloadComplete(downloadComplete)
{
}

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: sendFile
--
-- DATE:
--
-- REVISIONS:
--
-- DESIGNER: Calvin Rempel
--
-- PROGRAMMER: Calvin Rempel
--
-- INTERFACE: sendFile(char *filename, TCPSocket *socket)
--		char *filename	  : the name of the file to send.
--		TCPSocket *socket : the client to send the file to.
--
-- NOTES: Start sending a file to a client. Multiple file transfers can occur at once, both up
-- and down, from the same instance.
-------------------------------------------------------------------------------------------------*/
void FileTransferer::sendFile(SongName *song, TCPSocket *socket)
{
	int songId = song->id;
	char *path = song->cFilepath;
	char *filename = song->cFilename;

	FileTransferInfo *info = new FileTransferInfo;
	FileTransferData *data = new FileTransferData;
	info->pThis = this;
	info->socket = socket;
	info->data = data;

	memcpy(data->filename, filename, strlen(filename) + 1);
	memset(data->data, 0, FILE_PACKET_SIZE);
	data->f_SOF = true;
	data->f_EOF = false;
	data->dataLen = 0;
	data->songId = songId;

	if (filesOut.find(songId) == filesOut.end())
	{
		int i = errno;
		filesOut[songId][socket] = fopen(path, "rb");
		i = errno;
		i = GetLastError();
		transferring[songId][socket] = true;
		CreateThread(NULL, 0, FileTransferer::TransferThread, info, 0, NULL);
	}
}

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: recvFile
--
-- DATE:
--
-- REVISIONS:
--
-- DESIGNER: Calvin Rempel
--
-- PROGRAMMER: Calvin Rempel
--
-- INTERFACE: recvFile(char *data)
--		char *data : the received file data
--
-- NOTES: Data Received from a remote FileTranserer "sendFile" call. Data will be placed into
-- the file associated with the correct filename.
-------------------------------------------------------------------------------------------------*/
void FileTransferer::recvFile(char *data)
{
	FileTransferData *ft_data = (FileTransferData*) data;
	FILE *file;

	// Check if the file should be created
	if (ft_data->f_SOF)
	{
		//CreateDirectory(DOWNLOAD_FOLDER, NULL);
		filesIn[ft_data->songId] = fopen(ft_data->filename, "w");
	}

	file = filesIn[ft_data->songId];

	// If the file is open, add contents
	if (file)
	{
		// Write data into the file
		fwrite(ft_data->data, sizeof(char), ft_data->dataLen, file);

		// If End of File Sent, close the file
		if (ft_data->f_EOF)
		{
			fclose(file);
			//onDownloadComplete("", true);
		}
	}
}

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: cancelTransfer
--
-- DATE:
--
-- REVISIONS:
--
-- DESIGNER: Calvin Rempel
--
-- PROGRAMMER: Calvin Rempel
--
-- INTERFACE: cancelTransfer(char *filename, TCPSocket *socket)
--		char *filename    : the name of the file to cancel transferring
--		TCPSocket *socket : the socket of the client being transferred to.
--
-- NOTES: Stop a file Transfer to a given client.
-------------------------------------------------------------------------------------------------*/
void FileTransferer::cancelTransfer(int songId, TCPSocket *socket)
{
	transferring[songId][socket] = false;
	//onDownloadComplete("", false);
}

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: TransferThread
--
-- DATE:
--
-- REVISIONS:
--
-- DESIGNER: Calvin Rempel
--
-- PROGRAMMER: Calvin Rempel
--
-- INTERFACE: cancelTransfer(LPVOID transferInfo)
--		LPVOID transferInfo : the file transfer information
--
-- NOTES: Transfer a file in a thread until the file is completely sent, or transfer is
-- cancelled.
-------------------------------------------------------------------------------------------------*/
DWORD WINAPI FileTransferer::TransferThread(LPVOID transferInfo)
{
	FileTransferInfo *info = (FileTransferInfo*) transferInfo;
	FileTransferData *data = (FileTransferData*) info->data;

	FILE *file = info->pThis->filesOut[data->songId][info->socket];

	char buffer[FILE_PACKET_SIZE];
	bool success = false;
	int buffLen = 0;

	// Close if File is not opened
	if (!file)
		return 1;

	// Transfer data until end of file.
	while (info->pThis->transferring[data->songId][info->socket] && (buffLen = fread(buffer, 1, FILE_PACKET_SIZE, file)))
	{
		// Update the FileTransferInfo struct with new data
		data->dataLen = buffLen;
		memcpy(data->data, buffer, data->dataLen);

		// Send Data
		info->socket->Send(DOWNLOAD, (void*)data, sizeof(FileTransferData));

		// Mark all but first packet as NOT the start of file
		if (data->f_SOF)
		{
			data->f_SOF = false;
			success = true;
		}
	}

	// Send EOF packet
	data->f_EOF = true;
	info->socket->Send(DOWNLOAD, (void*)data, sizeof(FileTransferData));

	// Close the File
	fclose(file);
	//info->pThis->onDownloadComplete(data->filename, success);

	return 0;
}
