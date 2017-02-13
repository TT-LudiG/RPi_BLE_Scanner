#ifndef PACKET_CONNECT_H
#define PACKET_CONNECT_H

#include <string>

#include "Packet.h"

#define PACKET_TYPE_CONNECT 1

struct Packet_Connect: Packet
{
    std::string UnitNumber;
    std::string UnitLocation;

    Packet_Connect(unsigned char packetType = PACKET_TYPE_CONNECT): Packet(packetType){}
};

#endif