#ifndef PACKETBLE_H
#define PACKETBLE_H

#include <string>

struct PacketBLE
{
public:
    long int rssi;
    
    std::string payload;
    
    PacketBLE(long int rssi, std::string payload): rssi(rssi), payload(payload) {}
    ~PacketBLE() {}
};
#endif