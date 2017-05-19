#include <cerrno>
#include <cstring>

#include <arpa/inet.h>

#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <iostream>

#include "NetworkController_RPi.h"
#include "NetworkExceptions_RPi.h"

NetworkController_RPi::NetworkController_RPi(void): _nextSessionID(0) {}

NetworkController_RPi::~NetworkController_RPi(void)
{
    std::unordered_map<unsigned long int, SessionInfo*>::const_iterator it;
    
    for (it = _sessions.begin(); it != _sessions.end(); ++it)
        disconnectFromServer(it->first);
}

unsigned long int NetworkController_RPi::connectToServer(const std::string servername, const std::string port)
{
    long int socketHandle = -1;
    
    // Initialise the socket address.
    
    struct sockaddr_in* socketAddress;
    
    struct addrinfo* result;
    struct addrinfo hints;
    
    if ((getaddrinfo(servername.c_str(), port.c_str(), &hints, &result)) != 0)
    {
//        log_err("getaddrinfo failed: %s", gai_strerror(status));
//        return -1;
    }
    
    for (struct addrinfo* it = result; it != NULL; it = it->ai_next)
    {
        socketAddress = (struct sockaddr_in*)it->ai_addr;

        // Print the address string.
        
        char addressString[INET6_ADDRSTRLEN];
        
        inet_ntop(it->ai_family, &(socketAddress->sin_addr), addressString, sizeof(addressString));
        
        std::cout << std::string(addressString) << std::endl;
    }
    
    // Create the socket.
    
    socketHandle = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    
//    long int socketHandle = socket(PF_INET, SOCK_STREAM, 0);
    
    if (socketHandle < 0)
    {
        NetworkExceptions_RPi::SocketCreateException e(socketHandle, std::string(std::strerror(errno)));
        throw e;
    }
	
    // Connect to the server.
    
//    struct sockaddr_in socketAddress;
//	
//    initialiseSocketAddress(&socketAddress, servername.c_str(), port);
    
    // REINTERPRET_CAST!
	
    if (connect(socketHandle, reinterpret_cast<const struct sockaddr*>(socketAddress), sizeof(*socketAddress)) < 0)
    {       
        close(socketHandle);
        
//        NetworkExceptions_RPi::ServerConnectException e(servername, port, std::string(std::strerror(errno)));
//        throw e;
    }
    
    _sessions.insert(std::pair<unsigned long int, SessionInfo*>(_nextSessionID, new SessionInfo(socketHandle, *socketAddress)));
    
    return _nextSessionID++;
}

void NetworkController_RPi::disconnectFromServer(const unsigned long int sessionID)
{
    if (_sessions.count(sessionID) > 0)
    {
        close(_sessions.at(sessionID)->SocketHandle);
        
        delete _sessions.at(sessionID);
        
        _sessions.erase(sessionID);
    }
}

long int NetworkController_RPi::sendBufferWithSession(const unsigned long int sessionID, const unsigned char* inputBuffer, const unsigned long int bufferLength) const
{
    int bytesCount = -1;
    
    if (_sessions.count(sessionID) > 0)
    {
        long int socketHandle = _sessions.at(sessionID)->SocketHandle;
        
        bytesCount = write(socketHandle, static_cast<const void*>(inputBuffer), bufferLength);
        
        if (bytesCount < 0)
        {
            NetworkExceptions_RPi::SocketWriteException e(socketHandle, std::string(std::strerror(errno)));
            throw e;
        }
    }
    
    return bytesCount;
}

long int NetworkController_RPi::receiveBufferWithSession(const unsigned long int sessionID, unsigned char* outputBuffer, const unsigned long int bufferLength) const
{
    int bytesCount = -1;

    if (_sessions.count(sessionID) > 0)
    {
        long int socketHandle = _sessions.at(sessionID)->SocketHandle;
        
        bytesCount = read(socketHandle, static_cast<void*>(outputBuffer), bufferLength);
	
        if (bytesCount < 0)
        {
            NetworkExceptions_RPi::SocketReadException e(socketHandle, std::string(std::strerror(errno)));
            throw e;
        }
    }
    
    return bytesCount;
}