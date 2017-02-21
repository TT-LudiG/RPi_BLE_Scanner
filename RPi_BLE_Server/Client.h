#ifndef CLIENT_H
#define CLIENT_H

#include <string>

struct Client
{
    std::string ClientNumber;
    std::string ClientLocation;

    Client(std::string clientNumber, std::string clientLocation): ClientNumber(clientNumber), ClientLocation(clientLocation) {}
};

#endif