#ifndef PACKETBLE_H
#define PACKETBLE_H

#include <string>

// BLE beacon packet struct.

struct PacketBLE
{
public:
    float Value;
    float Battery;
    
    PacketBLE(float value, float battery): Value(value), Battery(battery) {}
    ~PacketBLE() {}
};
#endif