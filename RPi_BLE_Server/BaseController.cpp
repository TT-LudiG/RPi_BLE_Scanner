#include "stdafx.h"

#include <chrono>
#include <iostream>

#include "BaseController.h"

#include "Packet_Connect.h"
#include "Packet_Sensor.h"

BaseController::BaseController(): _networkControllerPtr(new NetworkController()), _clientIdNext(0) {}

BaseController::~BaseController()
{
    std::unordered_map<unsigned int, Client*>::iterator iterator1;

    for (iterator1 = _clients.begin(); iterator1 != _clients.end(); ++iterator1)
    {
        delete iterator1->second;
    }

    std::unordered_map<unsigned int, std::thread*>::iterator iterator2;

    for (iterator2 = _threads.begin(); iterator2 != _threads.end(); ++iterator2)
    {
        (*iterator2->second).join();

        delete iterator2->second;
    }

    delete _networkControllerPtr;
}

bool BaseController::update(void)
{
    // Get new clients.

    if (_networkControllerPtr->acceptNewClient(_clientIdNext))
    {
        _clients.insert(std::pair<unsigned int, Client*>(_clientIdNext, new Client()));

        _threads.insert(std::pair<unsigned int, std::thread*>(_clientIdNext, new std::thread(listenOnClient, _clientIdNext, _networkControllerPtr)));

        ++_clientIdNext;

        return true;
    }

    return false;
}

void BaseController::listenOnClient(unsigned int clientId, NetworkController* networkControllerPtr)
{
    char packetBuffer[SOCKET_BUFFER_SIZE_MAX];

    while (true)
    {
        // Get the data for this client.

        int packetLength = networkControllerPtr->receiveFromClient(clientId, packetBuffer);

        if (packetLength > 0)
        {
            int i = 0;

            while (i < packetLength)
            {
                unsigned int packetLength = SOCKET_BUFFER_SIZE_MAX;

                unsigned char packetType = static_cast<unsigned char>(packetBuffer[i]);

                if (packetType == PACKET_TYPE_DEFAULT)
                {
                    packetLength = sizeof(Packet);

                    Packet packet;

                    packet.deserialise(&(packetBuffer[i + 1]));

                    std::cout << static_cast<unsigned int>(packet.PacketType)
                                << "\nPACKET_DEFAULT_RECEIVED"
                                << std::endl;
                }

                else if (packetType == PACKET_TYPE_CONNECT)
                {
                    packetLength = sizeof(Packet_Connect);

                    Packet_Connect packet;

                    packet.deserialise(&(packetBuffer[i + 1]));

                    std::cout << static_cast<unsigned int>(packet.PacketType)
                                << "\nPACKET_CONNECT_RECEIVED"
                                << std::endl;
                }

                else if (packetType == PACKET_TYPE_SENSOR)
                {
                    packetLength = sizeof(Packet_Sensor);

                    Packet_Sensor packet;

                    packet.deserialise(&(packetBuffer[i + 1]));

                    std::cout << static_cast<unsigned int>(packet.PacketType)
                                << "\nPACKET_SENSOR_RECEIVED"
                                << std::endl;
                }

                else
                {
                    std::cout << "PACKET_ERROR" << std::endl;
                }

                i += packetLength + 1;
            }
        }

        // Sleep for one 1/10th of a second.

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
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