#include "stdafx.h"

#include "NetworkExceptions.h"
#include "NetworkController.h"

// Default constructor (error-prone).

NetworkController::NetworkController(const std::string port): _port(port), _socketGateway(INVALID_SOCKET)
{
    // Address info for the server to listen to.

    struct addrinfo* addrinfoResult = nullptr;
    struct addrinfo addrinfoHints;

    // Other declarations.

    WSADATA wsaData;

    int result = -1;

    // Initialise Winsock.

    result = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (result != 0)
    {
        NetworkExceptions::WinSock2Exception e;
        throw e;
    }

    // Set the address information.

    std::memset(&addrinfoHints, 0, sizeof(addrinfoHints));

    addrinfoHints.ai_family = AF_INET;
    addrinfoHints.ai_socktype = SOCK_STREAM;
    addrinfoHints.ai_protocol = IPPROTO_TCP; // TCP connection.
    addrinfoHints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port.

    result = getaddrinfo(NULL, _port.c_str(), &addrinfoHints, &addrinfoResult);

    if (result != 0)
    {
        WSACleanup();

        NetworkExceptions::ServerAddressException e;
        throw e;
    }

    // Create a SOCKET for connecting to the server.

    _socketGateway = socket(addrinfoResult->ai_family, addrinfoResult->ai_socktype, addrinfoResult->ai_protocol);

    if (_socketGateway == INVALID_SOCKET)
    {
        freeaddrinfo(addrinfoResult);

        WSACleanup();

        NetworkExceptions::SocketGatewayException e;
        throw e;
    }

    // Set the mode of the socket to be nonblocking. (*IS THIS CORRECT FOR OUR SYSTEM!?)

    unsigned long int mode = 1;

    result = ioctlsocket(_socketGateway, FIONBIO, &mode);

    if (result == SOCKET_ERROR)
    {
        closesocket(_socketGateway);

        WSACleanup();

        NetworkExceptions::BlockingNonException e;
        throw e;
    }

    // Set the gateway socket as a TCP socket.

    result = bind(_socketGateway, addrinfoResult->ai_addr, static_cast<int>(addrinfoResult->ai_addrlen));

    if (result == SOCKET_ERROR)
    {
        freeaddrinfo(addrinfoResult);

        closesocket(_socketGateway);

        WSACleanup();

        NetworkExceptions::BindTCPException e;
        throw e;
    }

    // Discard the address information.

    freeaddrinfo(addrinfoResult);

    // Start listening for new clients attempting to connect.

    result = listen(_socketGateway, SOMAXCONN);

    if (result == SOCKET_ERROR)
    {
        closesocket(_socketGateway);

        WSACleanup();

        NetworkExceptions::SocketListeningException e;
        throw e;
    }
}

// Default destructor.

NetworkController::~NetworkController(void)
{
    std::unordered_map<unsigned int, SOCKET>::iterator it;

    for (it = _clientSockets.begin(); it != _clientSockets.end(); ++it)
        closesocket(it->second);
}

// Method to attempt to accept a new client connection.

bool NetworkController::acceptNewClient(const unsigned int clientId)
{
    // If the client is waiting, accept the connection and save the socket.

    SOCKET socketClient = accept(_socketGateway, NULL, NULL);

    if (socketClient != INVALID_SOCKET)
    {
        // Disable Nagle's Algorithm on the client's socket.

        char value = 1;

        setsockopt(socketClient, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value));

        // Insert the new client into the session id table.

        _clientSockets.insert(std::pair<unsigned int, SOCKET>(clientId, socketClient));

        return true;
    }

    return false;
}

// Method to attempt to send outgoing data to a client.

int NetworkController::sendBufferToClient(const unsigned int clientId, unsigned char* outputBuffer, const unsigned long int bufferLength)
{
    int result = -1;

    if (_clientSockets.find(clientId) != _clientSockets.end())
    {
        SOCKET currentSocket = _clientSockets[clientId];

        result = send(currentSocket, reinterpret_cast<char*>(outputBuffer), bufferLength, 0);
    }

    return result;
}

// Method to attempt to receive incoming data from a client.

int NetworkController::receiveBufferFromClient(const unsigned int clientId, unsigned char* outputBuffer)
{
    int result = -1;

    if (_clientSockets.find(clientId) != _clientSockets.end())
    {
        SOCKET currentSocket = _clientSockets[clientId];

        result = recv(currentSocket, reinterpret_cast<char*>(outputBuffer), SOCKET_BUFFER_SIZE_MAX, 0);
    }

    return result;
}