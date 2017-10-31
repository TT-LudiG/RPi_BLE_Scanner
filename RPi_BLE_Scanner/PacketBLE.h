#ifndef PACKETBLE_H
#define PACKETBLE_H

#include <string>

struct PacketBLE
{
public:
    float Value;
    float Battery;
    
    PacketBLE(float value, float battery): Value(value), Battery(battery) {}
    ~PacketBLE() {}
};
#endif