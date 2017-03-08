#include "stdafx.h"

#include <iomanip>
#include <iostream>
#include <sstream>

#include "BaseController.h"

BaseController::BaseController(const unsigned short int port)
{
    _networkControllerPtr = new NetworkController(port);

    _sessionCount = 0;

    _isDone = false;

    _isReady = false;    
    _isWaitingCount = 0;
    _hasWokenCount = 0;
}

BaseController::~BaseController(void)
{
    {
        std::map<unsigned long int, bool>::iterator it;

        for (it = _sessions.begin(); it != _sessions.end(); ++it)
            _networkControllerPtr->closeSession(it->first);
    }

    {
        std::unordered_map<unsigned long int, std::thread*>::iterator it;

        for (it = _threads.begin(); it != _threads.end(); ++it)
            delete it->second;
    }

    delete _networkControllerPtr;
}

void BaseController::monitorThreads(void)
{
    std::unique_lock<std::mutex> lock(_mutex);

    std::unique_lock<std::mutex> lockSession(_mutexSession);

    lockSession.unlock();

    while (true)
    {
        // Join all session-handler threads if done.

        if (_isDone)
        {
            while (_isWaitingCount < _sessionCount) {}

            if (!_isReady)
            {
                _isReady = true;

                lock.unlock();

                _cv.notify_all();

                _isWaitingCount = 0;
            }

            std::unordered_map<unsigned long int, std::thread*>::const_iterator it;

            for (it = _threads.begin(); it != _threads.end(); ++it)
                it->second->join();

            break;
        }

        // Get new sessions.

        try
        {
            unsigned long int sessionID = _networkControllerPtr->getNewSession();

            _threads.emplace(sessionID, new std::thread(&BaseController::handleSession, this, sessionID));

            std::unique_lock<std::mutex> lock(_mutexSession);

            _sessions.emplace(sessionID, true);

            lock.unlock();

            ++_sessionCount;
        }

        catch (const std::exception&)
        {
            // Do nothing.
        }

        if (_sessionCount == 0)
            continue;

        // Remove any inactive sessions.

        lockSession.lock();

        std::map<unsigned long int, bool>::const_iterator it;

        for (it = _sessions.begin(); it != _sessions.end();)
        {
            if (!it->second)
            {
                std::thread* thread = _threads.at(it->first);

                thread->join();

                delete thread;

                _threads.erase(it->first);

                _networkControllerPtr->closeSession(it->first);

                _sessions.erase(it++);

                --_sessionCount;
            }

            else
                ++it;
        }

        lockSession.unlock();

        // Reset the conditional wait barrier for all session-handler threads.

        if ((_isReady) && (_hasWokenCount == _sessionCount))
        {
            lock.lock();

            _isReady = false;

            _hasWokenCount = 0;
        }

        // Force all synchronised session-handler threads to wait.

        if ((!_isReady) && (_isWaitingCount == _sessionCount))
        {
            std::clock_t start = std::clock();

            // All session-handler threads wait for 0.1 seconds (interruptible).

            while (((std::clock() - start) / CLOCKS_PER_SEC) < 0.1)
                if (_isDone)
                    break;

            _isReady = true;

            lock.unlock();

            _cv.notify_all();

            _isWaitingCount = 0;
        }
    }
}

void BaseController::handleSession(const unsigned long int sessionID)
{
    unsigned char buffer[SOCKET_BUFFER_SIZE_MAX];

    std::clock_t start = std::clock();

    while (!_isDone)
    {
        // Get the data for this client.

        long int bufferLength = _networkControllerPtr->receiveBufferWithSession(sessionID, buffer, SOCKET_BUFFER_SIZE_MAX);

        if (bufferLength > 0)
        {
            start = std::clock();

            long int currentIndex = 0;

            std::cout << "OK" << std::endl;

            /*while (i < bufferLength)
            {
                unsigned char messageLength;

                unsigned char temperatureLength;
                unsigned char humidityLength;
                unsigned char batteryLength;

                char id[6];

                short int temperature;    
                unsigned short int humidity;
                unsigned char battery;

                char time[20];

                unsigned int currentIndex = 0;

                memcpy(static_cast<void*>(&messageLength), buffer + currentIndex, 1);

                if (static_cast<int>(messageLength) > bufferLength)
                    break;

                currentIndex += 1;
                memcpy(static_cast<void*>(id), buffer + currentIndex, 6);

                currentIndex += 6;
                memcpy(static_cast<void*>(&temperatureLength), buffer + currentIndex, 1);
                currentIndex += 1;
                if (temperatureLength == sizeof(temperature))
                    memcpy(static_cast<void*>(&temperature), buffer + currentIndex, temperatureLength);

                currentIndex += temperatureLength;
                memcpy(static_cast<void*>(&humidityLength), buffer + currentIndex, 1);
                currentIndex += 1;
                if (humidityLength == sizeof(humidity))
                    memcpy(static_cast<void*>(&humidity), buffer + currentIndex, humidityLength);

                currentIndex += humidityLength;
                memcpy(static_cast<void*>(&batteryLength), buffer + currentIndex, 1);
                currentIndex += 1;
                if (batteryLength == sizeof(battery))
                    memcpy(static_cast<void*>(&battery), buffer + currentIndex, batteryLength);

                currentIndex += batteryLength;
                memcpy(static_cast<void*>(time), buffer + currentIndex, 20);

                std::stringstream idBuffer;

                idBuffer << std::hex << std::uppercase;

                for (unsigned int i = 0; i < 6; ++i)
                    idBuffer << std::setfill('0') << std::setw(2) << static_cast<int>(id[i]);

                std::cout << idBuffer.str() << "|" << temperature << "|" << humidity << "|" << static_cast<int>(battery) << "|" << time << std::endl;

                currentIndex += static_cast<int>(messageLength);
            }*/
        }

        else
        {
            // Break if no data has been received within SESSION_TIMEOUT seconds.

            if (((std::clock() - start) / CLOCKS_PER_SEC) > SESSION_TIMEOUT)
            {
                std::unique_lock<std::mutex> lock(_mutexSession);

                _sessions.at(sessionID) = false;

                break;
            }
        }

        ++_isWaitingCount;

        std::unique_lock<std::mutex> lock(_mutex);

        while (!_isReady)
            _cv.wait(lock);

        ++_hasWokenCount;
    }
}

void BaseController::finalise(void)
{
    _isDone = true;
}