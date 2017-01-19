#ifndef PACKET_SENSOR_RPI_H
#define PACKET_SENSOR_RPI_H

#include <ctime>

#include "Packet_RPi.h"

struct Packet_Sensor: Packet
{
    const unsigned char PacketType = 2;
    
    unsigned char ID[6];
    
    short Temperature;    
    unsigned short Humidity;
    unsigned char Battery;
    
    time_t Timestamp;
};

#endif