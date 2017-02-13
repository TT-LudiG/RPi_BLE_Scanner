#ifndef PACKET_RPI_H
#define PACKET_RPI_H

#include <cstring>

#define PACKET_TYPE_DEFAULT 0

struct Packet_RPi
{
    const unsigned char PacketType;
    
    Packet_RPi(unsigned char packetType = PACKET_TYPE_DEFAULT): PacketType(packetType){}
    
    void serialise(char* data) const
    {
        std::memcpy(data, (void*)(this), sizeof(Packet_RPi));
    }

    void deserialise(char* data) const
    {
        std::memcpy((void*)(this), data, sizeof(Packet_RPi));
    }
};

#endif