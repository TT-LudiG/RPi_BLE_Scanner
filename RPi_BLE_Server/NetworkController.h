#ifndef NETWORKCONTROLLER_H
#define NETWORKCONTROLLER_H

#include <string>
#include <unordered_map>

#include <ws2tcpip.h>

#include "Packet.h"

#define PORT "2226"

#define SOCKET_BUFFER_SIZE_MAX 1000000

class NetworkController
{
private:
    std::string _port;

    // Gateway socket for new client connections.

    SOCKET _socketGateway;

    // Dictionary to map an ID to each client socket.

    std::unordered_map<unsigned int, SOCKET> _clientSockets;

public:
    // Default constructor (error-prone).

    NetworkController(const std::string& port = PORT);

    // Default destructor.

    ~NetworkController(void);

    // Method to attempt to accept a new client connection.

    bool acceptNewClient(unsigned int clientId);

    // Method to attempt to receive incoming data from a client.

    int receiveFromClient(unsigned int clientId, char* outputBuffer);

    // Method to send outgoing data to a client (error-prone).

    //bool sendToClient(unsigned int clientId, const Packet& packet);
};

#endif