#ifndef PACKET_H
#define PACKET_H

#include <cstring>

#define PACKET_TYPE_DEFAULT 0

struct Packet
{
    const unsigned char PacketType;

    Packet(unsigned char packetType = PACKET_TYPE_DEFAULT): PacketType(packetType){}
    
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