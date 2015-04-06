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
void FileTransferer::sendFile(char *filename, TCPSocket *socket)
{
	FileTransferInfo info = { 0 };
	FileTransferData *data = new FileTransferData;
	info.pThis = this;
	info.socket = socket;
	info.data = data;

	memcpy(data->filename, filename, strlen(filename));
	memcpy(data->data, 0, FILE_PACKET_SIZE);
	data->f_SOF = true;
	data->f_EOF = false;
	data->dataLen = 0;

	if (filesOut.find(filename) != filesOut.end())
	{
		filesOut[filename][socket] = fopen(filename, "rb");
		transferring[filename][socket] = true;
		CreateThread(NULL, 0, FileTransferer::TransferThread, &info, 0, NULL);
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
		filesIn[ft_data->filename] = fopen(ft_data->filename, "wb");
	}

	file = filesIn[ft_data->filename];

	// If the file is open, add contents
	if (file)
	{
		// Write data into the file
		fwrite(ft_data->data, sizeof(char), ft_data->dataLen, file);

		// If End of File Sent, close the file
		if (ft_data->f_EOF)
		{
			fclose(file);
			onDownloadComplete(ft_data->filename, true);
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
void FileTransferer::cancelTransfer(char *filename, TCPSocket *socket)
{
	transferring[filename][socket] = false;
	onDownloadComplete(filename, false);
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

	std::ifstream file(data->filename, std::ios::binary);

	char buffer[FILE_PACKET_SIZE];
	bool success = false;

	// Transfer until cancelled or EOF is found.
	while (info->pThis->transferring[data->filename][info->socket] && file.is_open() && file.read(buffer, FILE_PACKET_SIZE))
	{
		// Update the FileTransferInfo struct with new data
		data->f_EOF = (file.eofbit) ? true : false;
		data->dataLen = file.gcount();
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

	// Close the File
	file.close();
	info->pThis->onDownloadComplete(data->filename, success);

	return 0;
}
