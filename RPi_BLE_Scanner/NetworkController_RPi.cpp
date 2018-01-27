#include <cerrno>
#include <cstring>

#include <arpa/inet.h>

#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>

#include "NetworkController_RPi.h"
#include "NetworkExceptions_RPi.h"

// Default constructor.

NetworkController_RPi::NetworkController_RPi(void): _nextSessionID(0) {}

// Destructor.

NetworkController_RPi::~NetworkController_RPi(void)
{
    disconnectFromServerAll();
}

// Method to create a connection session with a server (TCP).

unsigned long int NetworkController_RPi::connectToServer(const std::string servername, const std::string port)
{
    long int socketHandle = -1;
    
    // Initialise the socket address.
    
    struct sockaddr_in* socketAddress;
    
    struct addrinfo* result;
    
    unsigned long int status = getaddrinfo(servername.c_str(), port.c_str(), nullptr, &result);
    
    if (status != 0)
    {
        NetworkExceptions_RPi::ServerLookupException e(servername, std::string(gai_strerror(status)));
        throw e;
    }
    
    for (struct addrinfo* it = result; it != NULL; it = it->ai_next)
        socketAddress = (struct sockaddr_in*)it->ai_addr;
    
    // Create the socket.
    
    socketHandle = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    
    if (socketHandle < 0)
    {
        NetworkExceptions_RPi::SocketCreateException e(socketHandle, std::string(std::strerror(errno)));
        throw e;
    }
    
    // Free the memory allocated for the "addrinfo" struct.
    
    freeaddrinfo(result);
    
    // Set the socket timeout period (for the server-connect operation).
    
    struct timeval timeout;
    
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    
    // REINTERPRET_CAST!

    if (setsockopt(socketHandle, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<char*>(&timeout), sizeof(timeout)) < 0)
    {
        close(socketHandle);
        
        NetworkExceptions_RPi::SocketSetOptionException e(socketHandle, "SO_SNDTIMEO", std::string(std::strerror(errno)));
        throw e;
    }
    
    // REINTERPRET_CAST!
    
    if (setsockopt(socketHandle, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<char*>(&timeout), sizeof(timeout)) < 0)
    {
        close(socketHandle);
        
        NetworkExceptions_RPi::SocketSetOptionException e(socketHandle, "SO_RCVTIMEO", std::string(std::strerror(errno)));
        throw e;
    }
	
    // Connect to the server.
    
    // REINTERPRET_CAST!
	
    if (connect(socketHandle, reinterpret_cast<const struct sockaddr*>(socketAddress), sizeof(*socketAddress)) < 0)
    {
        close(socketHandle);
        
        NetworkExceptions_RPi::ServerConnectException e(servername, port, std::string(std::strerror(errno)));
        throw e;
    }
    
    // Set the socket timeout period (for subsequent read/write operations).
    
    timeout.tv_sec = 0;
    timeout.tv_usec = 1;
    
    // REINTERPRET_CAST!

    if (setsockopt(socketHandle, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<char*>(&timeout), sizeof(timeout)) < 0)
    {
        close(socketHandle);
        
        NetworkExceptions_RPi::SocketSetOptionException e(socketHandle, "SO_SNDTIMEO", std::string(std::strerror(errno)));
        throw e;
    }
    
    // REINTERPRET_CAST!
    
    if (setsockopt(socketHandle, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<char*>(&timeout), sizeof(timeout)) < 0)
    {
        close(socketHandle);
        
        NetworkExceptions_RPi::SocketSetOptionException e(socketHandle, "SO_RCVTIMEO", std::string(std::strerror(errno)));
        throw e;
    }
    
    _sessions.insert(std::pair<unsigned long int, SessionInfo*>(_nextSessionID, new SessionInfo(socketHandle, *socketAddress)));
    
    return _nextSessionID++;
}

// Method to disconnect a live connection session.

void NetworkController_RPi::disconnectFromServer(const unsigned long int sessionID)
{ 
    if (_sessions.count(sessionID) > 0)
    {      
        close(_sessions.at(sessionID)->SocketHandle);
        
        if (_sessions.at(sessionID) != nullptr)
            delete _sessions.at(sessionID);
        
        _sessions.erase(sessionID);
    }
}

// Method to disconnect all live connection sessions.

void NetworkController_RPi::disconnectFromServerAll(void)
{
    std::unordered_map<unsigned long int, SessionInfo*>::iterator it;
    
    for (it = _sessions.begin(); it != _sessions.end();)
    {     
        close(_sessions.at(it->first)->SocketHandle);
        
        if (_sessions.at(it->first) != nullptr)
            delete _sessions.at(it->first);
        
        it = _sessions.erase(it);
    }
}

// Method to attempt to send a byte stream over a connection session.

long int NetworkController_RPi::sendBufferWithSession(const unsigned long int sessionID, const unsigned char* inputBuffer, const unsigned long int bufferLength) const
{
    int bytesCount = -1;
    
    if (_sessions.count(sessionID) > 0)
    {
        long int socketHandle = _sessions.at(sessionID)->SocketHandle;
        
        bytesCount = send(socketHandle, static_cast<const void*>(inputBuffer), bufferLength, MSG_NOSIGNAL);
        
        if (bytesCount < 0)
        {
            if (errno == EAGAIN)
                bytesCount = 0;
            
            else
            {
                NetworkExceptions_RPi::SocketWriteException e(socketHandle, std::string(std::strerror(errno)));
                throw e;
            }
        }
    }
    
    return bytesCount;
}

// Method to attempt to read a byte stream from a connection session.

long int NetworkController_RPi::receiveBufferWithSession(const unsigned long int sessionID, unsigned char* outputBuffer, const unsigned long int bufferLength) const
{
    int bytesCount = -1;

    if (_sessions.count(sessionID) > 0)
    {
        long int socketHandle = _sessions.at(sessionID)->SocketHandle;
        
        bytesCount = read(socketHandle, static_cast<void*>(outputBuffer), bufferLength);
	
        if (bytesCount < 0)
        {
            if (errno == EAGAIN)
                bytesCount = 0;
            
            else
            {
                NetworkExceptions_RPi::SocketReadException e(socketHandle, std::string(std::strerror(errno)));
                throw e;
            }
        }
    }
    
    return bytesCount;
}