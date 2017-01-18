#ifndef PACKETTYPE_RPI_H
#define PACKETTYPE_RPI_H

enum class PacketType: unsigned char
{
    PACKET_NULL = 0,
    PACKET_CONNECT = 1,
    PACKET_SENSOR = 2
};

#endif