#ifndef _PACKET_TYPE_H_
#define _PACKET_TYPE_H_

#include <cstdint>
#include <vector>

#define MULTICAST_ADDR "224.0.0.1"

#define MULTICAST_PORT 7515

#define DATA_BUFSIZE 1024

#define SIZE_INDEX	4

#define STREAM_PACKET 9

#define STR_LEN 128

struct DataPacket
{
    int index;
    char data[DATA_BUFSIZE];
};

typedef struct DataPacket DataPacket;

struct StringPacket
{
    char string[STR_LEN];
};

typedef struct DataPacket DataPacket;

struct RequestPacket
{
    int index;
};

typedef struct RequestPacket RequestPacket;

struct MessageHeader
{
    uint32_t size;
    uint8_t type;
};

typedef struct MessageHeader MessageHeader;

struct SongStream
{
	short channels;
	short bps; //bit rae
	unsigned long sample_rate;
	int index;
	char songname[STR_LEN];
};

typedef struct SongStream SongStream;

struct SongName
{
	int index;
	char filepath[STR_LEN];	
};

typedef struct SongName SongName;

#endif
