#ifndef PACKET_RPI_H
#define PACKET_RPI_H

#include <cstring>

#include "PacketType_RPi.h"

#define MAX_PACKET_SIZE 1000000

struct Packet
{
    const PacketType type = PacketType::PACKET_NULL;
    
    void serialise(char* data) const
    {
        std::memcpy(data, (void*)(this), sizeof(Packet));
    }

    void deserialise(char* data) const
    {
        std::memcpy((void*)(this), data, sizeof(Packet));
    }
};

#endif