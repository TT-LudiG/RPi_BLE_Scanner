#include <cerrno>
#include <cstring>

#include <netdb.h>
#include <unistd.h>

#include <sys/socket.h>

#include "NetworkController_RPi.h"
#include "NetworkExceptions_RPi.h"
#include "Packet_Connect_RPi.h"
#include "Packet_RPi.h"

NetworkController::NetworkController(unsigned int port, std::string serverName): _packetSize(sizeof(Packet)), _serverName(serverName), _port(port)
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
    
    Packet_Connect packet;
	
    sendPacket(packet);
}

NetworkController::~NetworkController(void)
{
    close(_socket);
}

void NetworkController::initialiseSocketAddress(struct sockaddr_in* addressOutput, const char* hostname, uint16_t port)
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
	
    addressOutput->sin_addr = *(struct in_addr*)hostInfo->h_addr;
}

void NetworkController::sendPacket(const Packet& packet)
{	
    char packetBuffer[_packetSize];
	
    packet.serialise(packetBuffer);
	
    int bytesCount;

    bytesCount = write(_socket, packetBuffer, _packetSize);
	
    if (bytesCount < 0)
    {
        ServerWritexception e(std::string(std::strerror(errno)));
        throw e;
    }
}