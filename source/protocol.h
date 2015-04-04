#ifndef _PACKET_TYPE_H_
#define _PACKET_TYPE_H_

#define DATA_BUFSIZE 8192

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
    PacketType type;
    char data[DATA_BUFSIZE];
};

typedef struct DataPacket DataPacket;

struct CommandPacket
{
    PacketType type;
};

typedef struct CommandPacket CommandPacket;

#endif
