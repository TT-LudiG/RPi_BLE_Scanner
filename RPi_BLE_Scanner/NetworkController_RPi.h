#ifndef NETWORKCONTROLLER_H
#define NETWORKCONTROLLER_H

#include <string>

#include <netinet/in.h>

#include "Packet_RPi.h"

#define SERVERNAME "192.168.2.113"
#define PORT 2226

class NetworkController_RPi
{
private:
    std::string _serverName;
    unsigned short _port;
	
    int _socket;
	
    struct sockaddr_in _addressServer;
	
    static void initialiseSocketAddress(struct sockaddr_in* addressOutput, const char* hostname, unsigned short port);
	
public:
    NetworkController_RPi(const std::string& serverName = SERVERNAME, unsigned short port = PORT);
    
    ~NetworkController_RPi(void);
	
    void sendPacket(const Packet_RPi& packet);
};

#endif