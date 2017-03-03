#ifndef NETWORKCONTROLLER_H
#define NETWORKCONTROLLER_H

#include <string>

#include <netinet/in.h>

class NetworkController_RPi
{
private:
    const std::string _serverName;
    const unsigned short int _port;
	
    int _socket;
	
    struct sockaddr_in _addressServer;
	
    static void initialiseSocketAddress(struct sockaddr_in* addressOutput, const char* hostname, const unsigned short int port);
	
public:
    NetworkController_RPi(const std::string serverName, const unsigned short int port);
    
    ~NetworkController_RPi(void);
	
    int sendBuffer(unsigned char* inputBuffer, const unsigned long int bufferLength) const;
    
    int receiveBuffer(unsigned char* inputBuffer, const unsigned long int bufferLength) const;
};

#endif