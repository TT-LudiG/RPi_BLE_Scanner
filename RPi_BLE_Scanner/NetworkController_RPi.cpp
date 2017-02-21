#include <cerrno>
#include <cstring>

#include <netdb.h>
#include <unistd.h>

#include <sys/socket.h>

#include "NetworkController_RPi.h"
#include "NetworkExceptions_RPi.h"

NetworkController_RPi::NetworkController_RPi(const std::string serverName, const unsigned short int port): _serverName(serverName), _port(port)
{   
	// Create the socket.
	
    _socket = socket(PF_INET, SOCK_STREAM, 0);
	
    if (_socket < 0)
    {	
        SocketCreateException e(std::string(std::strerror(errno)));
        throw e;
    }
	
    // Connect to the server.
	
    initialiseSocketAddress(&_addressServer, _serverName.c_str(), _port);
	
    if (connect(_socket, (struct sockaddr*)&_addressServer, sizeof(_addressServer)) < 0)
    {      
        ServerConnectException e(std::string(std::strerror(errno)));
        throw e;
    }
}

NetworkController_RPi::~NetworkController_RPi(void)
{
    close(_socket);
}

void NetworkController_RPi::initialiseSocketAddress(struct sockaddr_in* addressOutput, const char* hostname, const unsigned short int port)
{
    struct hostent *hostInfo;

    addressOutput->sin_family = AF_INET;	
    addressOutput->sin_port = htons(port);
	
    hostInfo = gethostbyname(hostname);
	
    if (hostInfo == NULL)
    {
        HostnameLookupException e((std::string(hostname)));
        throw e;
    }
    
    // REINTERPRET_CAST!
	
    addressOutput->sin_addr = *reinterpret_cast<struct in_addr*>(hostInfo->h_addr);
}

void NetworkController_RPi::sendBuffer(unsigned char* inputBuffer, const unsigned long int bufferLength) const
{
    int bytesCount;

    bytesCount = write(_socket, static_cast<void*>(inputBuffer), bufferLength);
	
    if (bytesCount < 0)
    {
        ServerWriteException e(std::string(std::strerror(errno)));
        throw e;
    }
}