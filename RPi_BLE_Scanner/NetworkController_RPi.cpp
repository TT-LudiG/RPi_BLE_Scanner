#include <cerrno>

#include <netdb.h>
#include <unistd.h>

#include <sys/socket.h>

#include "NetworkController_RPi.h"
#include "NetworkExceptions_RPi.h"
#include "Packet_Connect_RPi.h"
#include "Packet_RPi.h"

NetworkController_RPi::NetworkController_RPi(const std::string& serverName, unsigned short port): _serverName(serverName), _port(port)
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
    
    Packet_Connect_RPi packet;
	
    sendPacket(packet);
}

NetworkController_RPi::~NetworkController_RPi(void)
{
    close(_socket);
}

void NetworkController_RPi::initialiseSocketAddress(struct sockaddr_in* addressOutput, const char* hostname, unsigned short port)
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

void NetworkController_RPi::sendPacket(const Packet_RPi& packet)
{	
    const unsigned int packetSize = sizeof(packet);
    
    char packetBuffer[packetSize + 1];
    
    // Set the first bit of the socket bit stream to the packet type.
    
    packetBuffer[0] = static_cast<char>(packet.PacketType);
    
    char packetBufferTemp[packetSize];
	
    packet.serialise(packetBufferTemp);
    
    std::memcpy(packetBuffer + 1, packetBufferTemp, packetSize);
	
    int bytesCount;

    bytesCount = write(_socket, packetBuffer, packetSize);
	
    if (bytesCount < 0)
    {
        ServerWritexception e(std::string(std::strerror(errno)));
        throw e;
    }
}