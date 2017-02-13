#ifndef PACKET_SENSOR_H
#define PACKET_SENSOR_H

#include <ctime>

#include "Packet.h"

#define PACKET_TYPE_SENSOR 2

struct Packet_Sensor: Packet
{ 
    unsigned char ID[6];
    
    short Temperature;    
    unsigned short Humidity;
    unsigned char Battery;
    
    time_t Timestamp;

    Packet_Sensor(unsigned char packetType = PACKET_TYPE_SENSOR): Packet(packetType){}
};

#endif