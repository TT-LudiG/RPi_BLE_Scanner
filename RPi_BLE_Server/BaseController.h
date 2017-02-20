#ifndef BASECONTROLLER_H
#define BASECONTROLLER_H

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

#include "Client.h"
#include "NetworkController.h"

class BaseController
{
private:
    NetworkController* _networkControllerPtr;

    std::unordered_map<unsigned int, Client*> _clients;

    std::unordered_map<unsigned int, std::thread*> _threads;

    unsigned int _clientCount = 0;

    std::atomic<bool> _isDone = false;

    std::mutex _mutex;
    std::atomic<bool> _isReady = false;
    std::atomic<unsigned int> _isWaitingCount = 0;
    std::atomic<unsigned int> _hasWokenCount = 0;

    std::condition_variable _cv;

    unsigned int _clientIdNext = 0;

    void listenOnClient(const unsigned int clientId);

public:
    BaseController(void);
    ~BaseController(void);

    void monitorClients(void);

    void finalise(void);

    //bool sendActionPacket(unsigned int clientId);
};

#endif