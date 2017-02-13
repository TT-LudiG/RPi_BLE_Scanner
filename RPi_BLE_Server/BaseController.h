#ifndef BASECONTROLLER_H
#define BASECONTROLLER_H

#include <thread>

#include "Client.h"
#include "NetworkController.h"

class BaseController
{
private:
    NetworkController* _networkControllerPtr;

    std::unordered_map<unsigned int, Client*> _clients;

    std::unordered_map<unsigned int, std::thread*> _threads;

    unsigned int _clientIdNext;

    static void listenOnClient(unsigned int clientId, NetworkController* networkControllerPtr);

public:
    BaseController(void);
    ~BaseController(void);

    bool update(void);

    //bool sendActionPacket(unsigned int clientId);
};

#endif