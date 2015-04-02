#ifndef _FILE_TRANSFERER_H_
#define _FILE_TRANSFERER_H_

#include <Windows.h>

#define FILENAME_PACKET_LENGTH 128
#define FILE_PACKET_SIZE 256

struct FileTransferInfo
{
	int socket;
	char *file;
};

class FileTransferer
{
	public:
		FileTransferer();
		~FileTransferer();
		
		void sendFile(int socket, char *file);

	private:
		static DWORD WINAPI TransferThread(LPVOID transferInfo);
};

#endif