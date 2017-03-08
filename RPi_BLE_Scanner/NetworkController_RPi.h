#ifndef NETWORKCONTROLLER_H
#define NETWORKCONTROLLER_H

#include <unordered_map>

#include "SessionInfo.h"

class NetworkController_RPi
{
private:
    std::unordered_map<unsigned long int, SessionInfo*> _sessions;
    
    unsigned long int _nextSessionID;
	
    static void initialiseSocketAddress(struct sockaddr_in* outputAddress, const char* hostname, const unsigned short int port);
	
public:
    NetworkController_RPi(void);    
    ~NetworkController_RPi(void);
    
    unsigned long int connectToServer(const std::string servername, const unsigned short int port);    
    void disconnectFromServer(const unsigned long int sessionID);
	
    long int sendBufferWithSession(const unsigned long int sessionID, const unsigned char* inputBuffer, const unsigned long int bufferLength) const;    
    long int receiveBufferWithSession(const unsigned long int sessionID, unsigned char* outputBuffer, const unsigned long int bufferLength) const;
};

#endif