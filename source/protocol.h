#ifndef _PACKET_TYPE_H_
#define _PACKET_TYPE_H_

#define DATA_BUFSIZE 8192

/*
 * length of a string
 */
#define STR_LEN 128

/*
 * packet types sent between clients and servers.
 */
enum class PacketType
{
    REQUEST_PACKET,
    REQUEST_DOWNLOAD,
    CANCEL_DOWNLOAD,
    DOWNLOAD_PACKET,
    RETRANSMISSION_PACKET,
    CHANGE_STREAM
};

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

struct CommandPacket
{
};

typedef struct CommandPacket CommandPacket;

struct RequestPacket
{
    int index;
};

typedef struct RequestPacket RequestPacket;

#endif
