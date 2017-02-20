#include "stdafx.h"

#include <iomanip>
#include <iostream>
#include <sstream>

#include "BaseController.h"

BaseController::BaseController(void): _networkControllerPtr(new NetworkController()) {}

BaseController::~BaseController(void)
{
    std::unordered_map<unsigned int, Client*>::iterator it1;

    for (it1 = _clients.begin(); it1 != _clients.end(); ++it1)
        delete it1->second;

    std::unordered_map<unsigned int, std::thread*>::iterator it2;

    for (it2 = _threads.begin(); it2 != _threads.end(); ++it2)
        delete it2->second;

    delete _networkControllerPtr;
}

void BaseController::monitorClients(void)
{
    std::unique_lock<std::mutex> lock(_mutex);

    while (!_isDone)
    {
        // Get new clients.

        if (_networkControllerPtr->acceptNewClient(_clientIdNext))
        {
            _clients.insert(std::pair<unsigned int, Client*>(_clientIdNext, new Client("NULL", "NULL")));

            _threads.insert(std::pair<unsigned int, std::thread*>(_clientIdNext, new std::thread(&BaseController::listenOnClient, this, _clientIdNext)));

            ++_clientCount;

            ++_clientIdNext;
        }

        if (_clientCount > 0)
        {
            // Reset the conditional wait barrier for all client-listener threads.

            if ((_isReady) && (_hasWokenCount == _clientCount))
            {
                lock.lock();

                _isReady = false;

                _hasWokenCount = 0;
            }

            // Force all synchronised client-listener threads to wait.

            if ((!_isReady) && (_isWaitingCount == _clientCount))
            {
                std::clock_t start = std::clock();

                // All client-listener threads wait for 0.1 seconds (interruptible).

                while (((std::clock() - start) / static_cast<double>(CLOCKS_PER_SEC)) < 0.1)
                    if (_isDone)
                        break;

                _isReady = true;

                lock.unlock();

                _cv.notify_all();

                _isWaitingCount = 0;
            }
        }
    }
}

void BaseController::listenOnClient(const unsigned int clientId)
{
    unsigned char buffer[SOCKET_BUFFER_SIZE_MAX];

    while (!_isDone)
    {
        // Get the data for this client.

        int bufferLength = _networkControllerPtr->receiveFromClient(clientId, buffer);

        if (bufferLength > 0)
        {
            int i = 0;

            while (i < bufferLength)
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

                if (messageLength > bufferLength)
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

                i += messageLength;
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

    std::unordered_map<unsigned int, std::thread*>::iterator it;

    for (it = _threads.begin(); it != _threads.end(); ++it)
        it->second->join();
}

//bool BaseController::sendActionPacket(unsigned int clientId)
//{
//    const unsigned int packetLength = sizeof(Packet);
//
//    char packetBuffer[packetLength];
//
//    Packet packet;
//    packet.type = PacketType::ACTION;
//
//    packet.serialise(packetBuffer);
//
//    return _networkControllerPtr->sendToClient(clientId, packetBuffer, packetLength);
//}