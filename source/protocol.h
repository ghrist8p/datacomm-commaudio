#ifndef _PACKET_TYPE_H_
#define _PACKET_TYPE_H_

#define DATA_BUFSIZE 8192

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

#endif
