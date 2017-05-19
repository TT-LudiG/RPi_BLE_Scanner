#include "stdafx.h"

#include "NetworkExceptions.h"
#include "NetworkController.h"

// Default constructor.

NetworkController::NetworkController(const unsigned short int port): _port(port), _socketGateway(INVALID_SOCKET)
{
    struct addrinfo* addrinfoResult = nullptr;
    struct addrinfo addrinfoHints;

    WSADATA wsaData;

    long int result = -1;

    // Initialise Winsock2.

    result = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (result != 0)
    {
        NetworkExceptions::WinSock2Exception e;
        throw e;
    }

    // Set the address information.

    std::memset(static_cast<void*>(&addrinfoHints), 0, sizeof(addrinfoHints));

    addrinfoHints.ai_family = AF_INET;
    addrinfoHints.ai_socktype = SOCK_STREAM;
    addrinfoHints.ai_protocol = IPPROTO_TCP; // TCP connection.
    addrinfoHints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port.

    result = getaddrinfo(NULL, std::to_string(_port).c_str(), &addrinfoHints, &addrinfoResult);

    if (result != 0)
    {
        WSACleanup();

        NetworkExceptions::ServerAddressException e;
        throw e;
    }

    // Get a gateway socket handle.

    _socketGateway = socket(addrinfoResult->ai_family, addrinfoResult->ai_socktype, addrinfoResult->ai_protocol);

    if (_socketGateway == INVALID_SOCKET)
    {
        freeaddrinfo(addrinfoResult);

        WSACleanup();

        NetworkExceptions::SocketGatewayException e;
        throw e;
    }

    // Set the gateway socket to be non-blocking mode.

    unsigned long int mode = 1;

    result = ioctlsocket(_socketGateway, FIONBIO, &mode);

    if (result == SOCKET_ERROR)
    {
        closesocket(_socketGateway);

        WSACleanup();

        NetworkExceptions::BlockingNonException e;
        throw e;
    }

    // Set the gateway socket to TCP mode.

    result = bind(_socketGateway, addrinfoResult->ai_addr, static_cast<long int>(addrinfoResult->ai_addrlen));

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
    std::unordered_map<unsigned long int, SOCKET>::const_iterator it;

    for (it = _sessions.begin(); it != _sessions.end(); ++it)
        closesocket(it->second);
}

// Method to get a new TCP session.

unsigned long int NetworkController::getNewSession(void)
{
    SOCKET socketHandle = accept(_socketGateway, NULL, NULL);

    if (socketHandle == INVALID_SOCKET)
    {
        NetworkExceptions::GetNewSessionException e;
        throw e;
    }

    // Disable Nagle's Algorithm.

    char value = 1;

    setsockopt(socketHandle, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value));

    _sessions.emplace(_nextSessionID, socketHandle);

    return _nextSessionID++;
}

// Method to close an existing TCP session.

void NetworkController::closeSession(const unsigned long int sessionID)
{
    if (_sessions.count(sessionID) > 0)
    {
        SOCKET currentSocket = _sessions.at(sessionID);

        closesocket(currentSocket);

        _sessions.erase(sessionID);
    }
}

// Method to send data, with a session.

long int NetworkController::sendBufferWithSession(const unsigned long int sessionID, const unsigned char* inputBuffer, const unsigned long int bufferLength) const
{
    long int result = -1;

    if (_sessions.count(sessionID) > 0)
    {
        SOCKET currentSocket = _sessions.at(sessionID);

        // REINTERPRET_CAST!

        result = send(currentSocket, reinterpret_cast<const char*>(inputBuffer), bufferLength, 0);
    }

    return result;
}

// Method to receive data, with a session.

long int NetworkController::receiveBufferWithSession(const unsigned long int sessionID, unsigned char* outputBuffer, const unsigned long int bufferLength) const
{
    long int result = -1;

    if (_sessions.count(sessionID) > 0)
    {
        SOCKET currentSocket = _sessions.at(sessionID);

        // REINTERPRET_CAST!

        result = recv(currentSocket, reinterpret_cast<char*>(outputBuffer), bufferLength, 0);
    }

    return result;
}