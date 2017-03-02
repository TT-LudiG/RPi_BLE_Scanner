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

    unsigned int _clientCount;

    std::atomic<bool> _isDone;

    std::mutex _mutex;
    std::atomic<bool> _isReady;
    std::atomic<unsigned int> _isWaitingCount;
    std::atomic<unsigned int> _hasWokenCount;

    std::condition_variable _cv;

    unsigned int _clientIdNext;

    void listenOnClient(const unsigned int clientId);

public:
    BaseController(void);
    ~BaseController(void);

    void monitorClients(void);

    void finalise(void);
};

#endif