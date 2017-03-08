#ifndef BASECONTROLLER_H
#define BASECONTROLLER_H

#include <atomic>
#include <condition_variable>
#include <map>
#include <mutex>
#include <thread>

#include "NetworkController.h"

#define SESSION_TIMEOUT 1

class BaseController
{
private:
    NetworkController* _networkControllerPtr;

    std::unordered_map<unsigned long int, std::thread*> _threads;

    std::mutex _mutexSession;
    std::map<unsigned long int, bool> _sessions;

    unsigned long int _sessionCount;

    std::atomic<bool> _isDone;

    std::mutex _mutex;
    std::atomic<bool> _isReady;
    std::atomic<unsigned long int> _isWaitingCount;
    std::atomic<unsigned long int> _hasWokenCount;

    std::condition_variable _cv;

    void handleSession(const unsigned long int sessionID);

public:
    BaseController(const unsigned short int port);
    ~BaseController(void);

    void monitorThreads(void);

    void finalise(void);
};

#endif