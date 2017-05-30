#include "stdafx.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "BaseController.h"

BaseController::BaseController(const unsigned short int port)
{
    _networkControllerPtr = nullptr;
    _mySQLControllerPtr = nullptr;

    _networkControllerPtr = new NetworkController(port);

    _mySQLControllerPtr = new MySQLController();

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

    if (_mySQLControllerPtr != nullptr)
        delete _mySQLControllerPtr;

    if (_networkControllerPtr != nullptr)
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

            std::stringstream bufferStream;

            // REINTERPRET_CAST!

            bufferStream.write(reinterpret_cast<const char*>(buffer), bufferLength);

            std::string currentLine;

            std::getline(bufferStream, currentLine);

            std::stringstream currentLineStream(currentLine);

            std::string currentField;

            std::getline(currentLineStream, currentField, '&');

            std::string id = currentField.substr(currentField.find('=') + 1);

            std::getline(currentLineStream, currentField, '&');

            unsigned long int timestamp = std::stoi(currentField.substr(currentField.find('=') + 1));

            std::getline(currentLineStream, currentField, '&');

            std::string data = currentField.substr(currentField.find('=') + 1);

            std::cout << data << std::endl;

            std::getline(currentLineStream, currentField, '&');

            std::string station = currentField.substr(currentField.find('=') + 1);

            std::cout << station << std::endl;

            std::getline(currentLineStream, currentField, '&');

            float latitude = std::stof(currentField.substr(currentField.find('=') + 1));

            std::cout << latitude << std::endl;

            std::getline(currentLineStream, currentField, '&');

            float longitude = std::stof(currentField.substr(currentField.find('=') + 1));

            std::cout << longitude << std::endl;

            std::getline(currentLineStream, currentField, '&');

            float noiseAverage = std::stof(currentField.substr(currentField.find('=') + 1));

            std::cout << noiseAverage << std::endl;

            std::getline(currentLineStream, currentField, '&');

            float rssi = std::stof(currentField.substr(currentField.find('=') + 1));

            std::cout << rssi << std::endl;

            std::getline(currentLineStream, currentField, '&');

            unsigned long int sequenceNo = std::stoi(currentField.substr(currentField.find('=') + 1));

            std::cout << sequenceNo << std::endl;
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