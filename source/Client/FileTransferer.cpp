#include "FileTransferer.h"

#include <fstream>
#include <string>

FileTransferer::FileTransferer(char *filename, int socket, bool send)
	: filename(filename), socket(socket)
{
	if (send)
		sendFile();
}

FileTransferer::~FileTransferer()
{

}

void FileTransferer::sendFile()
{
	FileTransferInfo info = { 0 };
	info.pThis = this;
	memcpy(info.filename, file, strlen(filename));
	memcpy(info.data, 0, FILE_PACKET_SIZE);
	info.f_SOF = true;
	info.f_EOF = false;
	info.dataLen = 0;
	info.socket = socket;

	transferring = true;
	CreateThread(NULL, 0, FileTransferer::TransferThread, &info, 0, NULL);
}

void FileTransferer::recvFile(char *data)
{
	FileTransferInfo *info = (FileTransferInfo*) data;

	// Check if the file should be created
	if (info->f_SOF)
	{
		//CreateDirectory(DOWNLOAD_FOLDER, NULL);
		file = fopen(info->filename, "wb");
	}

	// If the file is open, add contents
	if (file)
	{
		// Write data into the file
		fwrite(info->data, sizeof(char), info->dataLen, file);

		// If End of File Sent, close the file
		if (info->f_EOF)
		{
			fclose(file);
		}
	}
}

void FileTransferer::cancelTransfer()
{
	transferring = false;
}

DWORD WINAPI FileTransferer::TransferThread(LPVOID transferInfo)
{
	FileTransferInfo *info = (FileTransferInfo*) transferInfo;
	std::ifstream file(info->filename, std::ios::binary);

	char buffer[FILE_PACKET_SIZE];

	while (info->pThis->transferring && file.is_open() && file.read(buffer, FILE_PACKET_SIZE))
	{
		// Update the FileTransferInfo struct with new data
		info->f_EOF = (file.eofbit) ? true : false;
		info->dataLen = file.gcount();
		memcpy(info->data, buffer, info->dataLen);

		// Send Data
		send(info->socket, (char*) info, sizeof(FILE_PACKET_SIZE), NULL);

		// Mark all but first packet as NOT the start of file
		if (info->f_SOF)
			info->f_SOF = false;
	}

	file.close();

	return 0;
}
