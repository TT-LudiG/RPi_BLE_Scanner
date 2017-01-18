#ifndef NETWORKCONTROLLER_H
#define NETWORKCONTROLLER_H

#include <string>

#include <netinet/in.h>

#include "Packet_RPi.h"

#define SERVERNAME "192.168.2.113"
#define PORT 2226

class NetworkController
{
private:
    std::string _serverName;
    unsigned int _port;
	
    int _socket;
	
    struct sockaddr_in _addressServer;
	
    const unsigned int _packetSize;
	
    static void initialiseSocketAddress(struct sockaddr_in* addressOutput, const char* hostname, uint16_t port);
	
public:
    NetworkController(unsigned int port = PORT, std::string serverName = SERVERNAME);
    ~NetworkController(void);
	
    void sendPacket(const Packet& packet);
};

#endif