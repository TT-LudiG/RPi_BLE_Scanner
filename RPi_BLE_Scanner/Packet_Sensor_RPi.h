#ifndef PACKET_SENSOR_RPI_H
#define PACKET_SENSOR_RPI_H

#include <bitset>
#include <string>

#include "Packet_RPi.h"

struct Packet_Sensor: Packet
{
    const PacketType type = PacketType::PACKET_SENSOR;
    
    
    
    std::bitset<10> Temperature;    
    std::bitset<9> Humidity;    
    std::bitset<7> Battery;
};

#endif