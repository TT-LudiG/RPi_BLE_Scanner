#ifndef PACKET_CONNECT_RPI_H
#define PACKET_CONNECT_RPI_H

#include "Packet_RPi.h"

struct Packet_Connect: Packet
{
    const unsigned char PacketType = 1;
};

#endif