#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#include <cstdint>
#include <vector>

#define MULTICAST_ADDR "224.0.0.1"

#define MULTICAST_PORT 7515

#define DATA_BUFSIZE 8196

#define SIZE_INDEX	4

#define STREAM_PACKET 9

#define DATA_LEN 60

#define STR_LEN 128

/**
 * audio data packet, that has an {index}, describing in what order the packet is
 *   supposed to be played.
 *
 * also has a {data} member, used to hold the raw PCM data
 */
struct DataPacket
{
    int index;
    char data[DATA_LEN];
};

typedef struct DataPacket DataPacket;

/**
 * local data packet is used internal to the client. its like a data packet, but
 *   has an extra member for storing the packet's source address.
 *
 * {index}; describing in what order the packet is supposed to be played in.
 *
 * {srcAddr}; holds the source address that sent this packet
 *
 * {data}; raw PCM data to play
 */
struct LocalDataPacket
{
    int index;
    unsigned long srcAddr;
    char data[DATA_LEN];
};

typedef struct LocalDataPacket LocalDataPacket;

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
	int id;
	char songname[STR_LEN];
};

typedef struct SongStream SongStream;

struct SongName
{
	int  id;
	char filepath[STR_LEN];
};

typedef struct SongName SongName;

#endif
