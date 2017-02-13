#ifndef PACKET_CONNECT_RPI_H
#define PACKET_CONNECT_RPI_H

#include <string>

#include "Packet_RPi.h"

#define PACKET_TYPE_CONNECT 1

struct Packet_Connect_RPi: Packet_RPi
{  
    std::string UnitNumber;
    std::string UnitLocation;
    
    Packet_Connect_RPi(unsigned char packetType = PACKET_TYPE_CONNECT): Packet_RPi(packetType){}
};

#endif