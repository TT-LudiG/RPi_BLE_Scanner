#ifndef NETWORKCONTROLLER_H
#define NETWORKCONTROLLER_H

#include <string>
#include <unordered_map>

#include <ws2tcpip.h>

// Set the max socket buffer size to the MTU (Maximum Transmission Unit).

#define SOCKET_BUFFER_SIZE_MAX 1400

class NetworkController
{
private:
    // Map of session IDs to socket handles.

    std::unordered_map<unsigned long int, SOCKET> _sessions;

    // Port to listen on for incoming TCP connections.

    const unsigned short int _port;

    // Gateway socket for incoming TCP connections.

    SOCKET _socketGateway;

    unsigned long int _nextSessionID;

public:
    NetworkController(const unsigned short int port);
    ~NetworkController(void); 

    unsigned long int getNewSession(void);
    void closeSession(const unsigned long int sessionID);

    long int sendBufferWithSession(const unsigned long int sessionID, const unsigned char* inputBuffer, const unsigned long int bufferLength) const;
    long int receiveBufferWithSession(const unsigned long int sessionID, unsigned char* outputBuffer, const unsigned long int bufferLength) const;
};

#endif