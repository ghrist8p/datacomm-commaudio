#ifndef _FILE_TRANSFERER_H_
#define _FILE_TRANSFERER_H_

#include "../common.h"
#include <map>

#define FILENAME_PACKET_LENGTH 128
#define FILE_PACKET_SIZE 256

class FileTransferer;

struct FileTransferInfo
{
	FileTransferer *pThis;
	int socket;
	char filename[FILENAME_PACKET_LENGTH];
	char data[FILE_PACKET_SIZE];
	int dataLen;
	bool f_SOF;
	bool f_EOF;
};

class FileTransferer
{
	public:
		FileTransferer(char *filename, int socket, bool send);
		~FileTransferer();

		void sendFile();
		void recvFile(char *data);
		void cancelTransfer();

	private:
		static DWORD WINAPI TransferThread(LPVOID transferInfo);

		int currentSegment;
		FILE *file;
		char *filename;
		int socket;
		bool transferring;
};

#endif
