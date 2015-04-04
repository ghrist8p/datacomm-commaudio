#include "FileTransferer.h"

#include <fstream>

FileTransferer::FileTransferer()
{

}

FileTransferer::~FileTransferer()
{

}

void FileTransferer::sendFile(int socket, char *file)
{
    FileTransferInfo info = {0};
	memoryCopy(info.file, file, strlen(file));
	info.socket = socket;

	CreateThread(NULL, 0, FileTransferer::TransferThread, &info, 0, NULL);
}

DWORD WINAPI FileTransferer::TransferThread(LPVOID transferInfo)
{
	FileTransferInfo *info = (FileTransferInfo*) transferInfo;

	std::ifstream file(info->file, std::ios::binary);

	if (file.is_open())
	{
		// Send Filename
		send(info->socket, info->file, FILENAME_PACKET_LENGTH, NULL);

		// Send FileLength

		// Read data from file
		char buffer[FILE_PACKET_SIZE];
		while (file.read(buffer, FILE_PACKET_SIZE))
		{
			send(info->socket, buffer, FILE_PACKET_SIZE, NULL);
		}
	}
	else
	{
		send(info->socket, '\0', 1, NULL);
	}

	return 0;
}
