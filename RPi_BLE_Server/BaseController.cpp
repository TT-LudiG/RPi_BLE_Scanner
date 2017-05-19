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

            //try
            //{
            //    _mySQLControllerPtr->insertValuesRaw("5F87RF", 1493299361, "45854X2S5S12", "59172", -23.5, 12.0, 74.34, -187.00, 52);
            //}

            //catch (const sql::SQLException&)
            //{
            //    // Do nothing.
            //}

            //long int currentIndex = 0;

            //while (currentIndex < bufferLength)
            //{
            //    unsigned char messageLength;

            //    unsigned char temperatureLength;
            //    unsigned char humidityLength;
            //    unsigned char batteryLength;

            //    unsigned char conduitNameLength;

            //    char id[6];

            //    short int temperature;
            //    unsigned short int humidity;
            //    unsigned char battery;

            //    char time[20];

            //    char conduitName[50];

            //    memcpy(static_cast<void*>(&messageLength), static_cast<const void*>(buffer + currentIndex), 1);

            //    if ((static_cast<int>(messageLength) + 1) > bufferLength)
            //        break;

            //    currentIndex += 1;
            //    memcpy(static_cast<void*>(id), static_cast<const void*>(buffer + currentIndex), 6);

            //    currentIndex += 6;
            //    memcpy(static_cast<void*>(&temperatureLength), static_cast<const void*>(buffer + currentIndex), 1);
            //    currentIndex += 1;
            //    if (temperatureLength == sizeof(temperature))
            //        memcpy(static_cast<void*>(&temperature), static_cast<const void*>(buffer + currentIndex), temperatureLength);

            //    currentIndex += temperatureLength;
            //    memcpy(static_cast<void*>(&humidityLength), static_cast<const void*>(buffer + currentIndex), 1);
            //    currentIndex += 1;
            //    if (humidityLength == sizeof(humidity))
            //        memcpy(static_cast<void*>(&humidity), static_cast<const void*>(buffer + currentIndex), humidityLength);

            //    currentIndex += humidityLength;
            //    memcpy(static_cast<void*>(&batteryLength), static_cast<const void*>(buffer + currentIndex), 1);
            //    currentIndex += 1;
            //    if (batteryLength == sizeof(battery))
            //        memcpy(static_cast<void*>(&battery), static_cast<const void*>(buffer + currentIndex), batteryLength);

            //    currentIndex += batteryLength;
            //    memcpy(static_cast<void*>(time), static_cast<const void*>(buffer + currentIndex), 20);

            //    currentIndex += 20;
            //    memcpy(static_cast<void*>(&conduitNameLength), static_cast<const void*>(buffer + currentIndex), 1);
            //    currentIndex += 1;
            //    memcpy(static_cast<void*>(&conduitName), static_cast<const void*>(buffer + currentIndex), conduitNameLength);

            //    std::stringstream idBuffer;

            //    idBuffer << std::hex << std::uppercase;

            //    for (unsigned int i = 0; i < 6; ++i)
            //        idBuffer << std::setfill('0') << std::setw(2) << static_cast<int>(id[i]);

            //    std::string conduitNameString(conduitName, conduitNameLength);

            //    std::ofstream fileLog("LOG_BLE.txt", std::ofstream::app);

            //    if (fileLog.is_open())
            //    {
            //        fileLog << conduitNameString << ": " << idBuffer.str() << "|" << temperature << "|" << humidity << "|" << static_cast<int>(battery) << "|" << time << std::endl;

            //        fileLog.close();
            //    }

            //    //std::cout << conduitNameString << ": " << idBuffer.str() << "|" << temperature << "|" << humidity << "|" << static_cast<int>(battery) << "|" << time << std::endl;

            //    currentIndex += static_cast<int>(messageLength) + 1;
            //}
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