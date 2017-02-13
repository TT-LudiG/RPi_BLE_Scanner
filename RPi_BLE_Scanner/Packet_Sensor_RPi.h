#ifndef PACKET_SENSOR_RPI_H
#define PACKET_SENSOR_RPI_H

#include <ctime>

#include "Packet_RPi.h"

#define PACKET_TYPE_SENSOR 2

struct Packet_Sensor_RPi: Packet_RPi
{    
    unsigned char ID[6];
    
    short Temperature;    
    unsigned short Humidity;
    unsigned char Battery;
    
    time_t Timestamp;
    
    Packet_Sensor_RPi(unsigned char packetType = PACKET_TYPE_SENSOR): Packet_RPi(packetType){}
};

#endif