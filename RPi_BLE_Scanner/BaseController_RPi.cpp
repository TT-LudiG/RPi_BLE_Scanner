#include <algorithm>
#include <chrono>
#include <iostream>
#include <thread>

#include "BaseController_RPi.h"

const std::string BaseController_RPi::_base64Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

BaseController_RPi::BaseController_RPi(void)
{
    _networkControllerPtr = new NetworkController_RPi();
    
    _bluetoothControllerPtr = new BluetoothController();
    
    _isDone = false;
}

BaseController_RPi::BaseController_RPi(std::string serverName, unsigned int port)
{
    _networkControllerPtr = new NetworkController_RPi(serverName, port);
    
    _bluetoothControllerPtr = new BluetoothController();
    
    _isDone = false;
}

BaseController_RPi::~BaseController_RPi(void)
{
    std::map<std::string, BeaconState*>::const_iterator it;
    
    for (it = _beacons.begin(); it != _beacons.end(); ++it)
        delete it->second;
    
    delete _networkControllerPtr;
    
    _bluetoothControllerPtr->stopHCIScan_BLE();
    
    _bluetoothControllerPtr->closeHCIDevice();
    
    delete _bluetoothControllerPtr;
}

void BaseController_RPi::sendDataPeriodically(void) const
{
    while (!_isDone)
    {
        std::map<std::string, BeaconState*>::const_iterator it;
        
        for (it = _beacons.begin(); it != _beacons.end(); ++it)
        {
            char id[6];
            
            std::string idString = it->first;
            
            for (unsigned int i = 0; i < 6; ++i)
                id[i] = static_cast<char>(std::stoul(idString.substr((2 * i), 2), nullptr, 16));
            
            BeaconState state = *it->second;
            
            time_t timeRaw;
            
            time(&timeRaw);
            
            struct tm timeInfo = *localtime(&timeRaw);
            
            char time[20];
            
            strftime(time, 20, "%F %T", &timeInfo);
            
            unsigned char temperatureLength = sizeof(state.Temperature);
            unsigned char humidityLength = sizeof(state.Humidity);
            unsigned char batteryLength = sizeof(state.Battery);
            
            unsigned char bufferLength = 1 + 6 + 1 + temperatureLength + 1 + humidityLength + 1 + batteryLength + 20;
            
            unsigned char buffer[bufferLength];
            
            unsigned int currentIndex = 0;
            
            memcpy(buffer + currentIndex, static_cast<const void*>(&bufferLength), 1);
            
            currentIndex += 1;
            memcpy(buffer + currentIndex, static_cast<const void*>(id), 6);
            
            currentIndex += 6;
            memcpy(buffer + currentIndex, static_cast<const void*>(&temperatureLength), 1);
            currentIndex += 1;
            memcpy(buffer + currentIndex, static_cast<const void*>(&state.Temperature), temperatureLength);
            
            currentIndex += temperatureLength;
            memcpy(buffer + currentIndex, static_cast<const void*>(&humidityLength), 1);
            currentIndex += 1;
            memcpy(buffer + currentIndex, static_cast<const void*>(&state.Humidity), humidityLength);
            
            currentIndex += humidityLength;
            memcpy(buffer + currentIndex, static_cast<const void*>(&batteryLength), 1);
            currentIndex += 1;
            memcpy(buffer + currentIndex, static_cast<const void*>(&state.Battery), batteryLength);
            
            currentIndex += batteryLength;
            memcpy(buffer + currentIndex, static_cast<const void*>(time), 20);
            
            std::cout << idString << "|" << state.Temperature << "|" << state.Humidity << "|" << static_cast<int>(state.Battery) << "|" << time << std::endl;
            
            _networkControllerPtr->sendBuffer(buffer, bufferLength);
            
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

void BaseController_RPi::listenforBLEDevices(void)
{	
    _bluetoothControllerPtr->startHCIScan_BLE();
    
    std::cout << "Scanning..." << std::endl;
	
    while (!_isDone)
    {
        unsigned char inputBuffer[HCI_MAX_EVENT_SIZE];
	    
        if ((_bluetoothControllerPtr->readDeviceInput(inputBuffer, HCI_MAX_EVENT_SIZE)) < 0)
        {
            if (errno == EAGAIN)
                std::this_thread::sleep_for(std::chrono::microseconds(100));
            
            else
            {
                if (errno == EINTR)
                    std::cerr << "Interrupted system call." << std::endl;
                
                std::cout << "Scanning error." << std::endl;
                
                break;
            }
        }

        else
        {
            // REINTERPRET_CAST!
            
            evt_le_meta_event* metaPtr = reinterpret_cast<evt_le_meta_event*>(inputBuffer + (1 + HCI_EVENT_HDR_SIZE));
            
            if (metaPtr->subevent != EVT_LE_ADVERTISING_REPORT)
                continue;
            
            uint8_t reportCount = metaPtr->data[0];
        
            void* offsetPtr = metaPtr->data + 1;
            
            while (reportCount--)
            {    
                le_advertising_info* infoPtr = static_cast<le_advertising_info*>(offsetPtr);
                
                uint8_t dataLength = infoPtr->length;

                if (dataLength == 0)
                    continue;
                
                // Manufacturer data is found from indices 2-4 (length: 3).
                
                unsigned char manufacturerData[3];
                
                memcpy(manufacturerData, (infoPtr->data + 2), 3);
                
                std::string manufacturer(manufacturerData, manufacturerData + 3);
                
                if (manufacturer == "INO")
                {
                    // Temperature data is found from indices 10-22 (length: 13).
                
                    unsigned char payloadData[12];
                
                    memcpy(payloadData, (infoPtr->data + 11), 12);
                
                    std::string payload = base64Decode(payloadData, 12);
                    
                    if (payload.size() == 9)
                    {
                        char idBuffer[6];
                        
                        ba2str(&infoPtr->bdaddr, idBuffer);
                        
                        std::string id = idBuffer;
                        
                        id.erase(std::remove(id.begin(), id.end(), ':'), id.end());
                        
//                        int8_t rssi = infoPtr->data[infoPtr->length];
                        
                        short int temperature = getTemperature(payload.substr(0, 4));
                        unsigned short int humidity = std::stoi(payload.substr(4, 3)) & 0xFFFF;
                        unsigned char battery = std::stoi(payload.substr(7, 2)) & 0xFF;
                        
                        if (_beacons.count(id) == 0)                           
                            _beacons.emplace(id, new BeaconState(temperature, humidity, battery));
                        
                        else
                        {
                            _beacons[id]->Temperature = temperature;
                            _beacons[id]->Humidity = humidity;
                            _beacons[id]->Battery = battery;
                        }
                    }
                }
        
                offsetPtr = infoPtr->data + (infoPtr->length + 2);
            }
        }
    }
    
    _bluetoothControllerPtr->stopHCIScan_BLE();
}

void BaseController_RPi::finalise(void)
{
    _isDone = true;
}

short BaseController_RPi::getTemperature(const std::string& temperatureStr)
{
    short int temperature = (std::stoi(temperatureStr.substr(1, 3)) & 0xFFFF);
    
    if (temperatureStr[0] == '-')
        temperature *= -1;
    
    return temperature;
}

// The "isBase64" and "base64Decode" functions were based off of 3rd-party source code, distributed under the following license:

/* 
   base64.cpp and base64.h

   Copyright (C) 2004-2008 René Nyffenegger

   This source code is provided 'as-is', without any express or implied
   warranty. In no event will the author be held liable for any damages
   arising from the use of this software.

   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely, subject to the following restrictions:

   1. The origin of this source code must not be misrepresented; you must not
      claim that you wrote the original source code. If you use this source code
      in a product, an acknowledgment in the product documentation would be
      appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
      misrepresented as being the original source code.

   3. This notice may not be removed or altered from any source distribution.

   René Nyffenegger rene.nyffenegger@adp-gmbh.ch

*/

bool BaseController_RPi::isBase64(unsigned char inputChar)
{
    return (isalnum(inputChar) || (inputChar == '+') || (inputChar == '/'));
}

std::string BaseController_RPi::base64Decode(const unsigned char* inputBuffer, const unsigned long int bufferLength)
{    
    std::string decodedString;
    
    unsigned char temp1[4];
    unsigned char temp2[3];
    
    int i = 0;
    
    int index = 0;
    
    // Handle data in sets of 4.
    
    unsigned long int currentLength = bufferLength;

    while ((currentLength--) && (inputBuffer[index] != '=') && (isBase64(inputBuffer[index])))
    {       
        temp1[i++] = inputBuffer[index];
        
        ++index;
        
        if (i == 4)
        {      
            for (i = 0; i < 4; ++i)
                temp1[i] = _base64Chars.find(temp1[i]);

            temp2[0] = (temp1[0] << 2) + ((temp1[1] & 0x30) >> 4);
            temp2[1] = ((temp1[1] & 0xf) << 4) + ((temp1[2] & 0x3c) >> 2);
            temp2[2] = ((temp1[2] & 0x3) << 6) + temp1[3];

            for (i = 0; i < 3; ++i)      
                decodedString += temp2[i];
        
            i = 0;
        }
    }
    
    // Handle an incomplete final set (>4).

    if (i > 0)
    {
        for (int j = i; j < 4; ++j)
            temp1[j] = 0;

        for (int j = 0; j < 4; ++j)
            temp1[j] = _base64Chars.find(temp1[j]);
        
        temp2[0] = (temp1[0] << 2) + ((temp1[1] & 0x30) >> 4);
        temp2[1] = ((temp1[1] & 0xf) << 4) + ((temp1[2] & 0x3c) >> 2);
        temp2[2] = ((temp1[2] & 0x3) << 6) + temp1[3];

        for (int j = 0; j < (i - 1); ++j)           
            decodedString += temp2[j];
    }

    return decodedString;
}