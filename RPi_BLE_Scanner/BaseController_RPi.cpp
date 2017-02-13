#include <algorithm>
#include <chrono>
#include <iostream>
#include <thread>

#include "BaseController_RPi.h"
#include "Packet_Sensor_RPi.h"

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
    delete _networkControllerPtr;
    
    _bluetoothControllerPtr->stopHCIScan_BLE();
    
    _bluetoothControllerPtr->closeHCIDevice();
    
    delete _bluetoothControllerPtr;
}

void BaseController_RPi::sendDataPeriodically(void)
{
    while (!_isDone)
    {
        for (std::map<std::string, BeaconState>::const_iterator it = _beacons.begin(); it != _beacons.end(); ++it)
        {
            Packet_Sensor_RPi packet;
            
            memcpy(packet.ID, it->first.c_str(), 6);
            
            packet.Temperature = it->second.Temperature;
            packet.Humidity = it->second.Humidity;
            packet.Battery = it->second.Battery;
            
            time_t timeRaw;
            
            time(&timeRaw);
            
            packet.Timestamp = timeRaw;
            
            struct tm * timeInfo = localtime(&packet.Timestamp);
            
            char timeBuffer[80];
            
            strftime(timeBuffer, 80, "%F %T", timeInfo);
            
            std::cout << it->first << "|" << packet.Temperature << "|" << packet.Humidity << "|" << int(packet.Battery) << "|" << timeBuffer << std::endl;
            
            _networkControllerPtr->sendPacket(packet);
        }
        
        std::cout << "***" << std::endl;
        
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

void BaseController_RPi::listenforBLEDevices(void)
{	
    _bluetoothControllerPtr->startHCIScan_BLE();
    
    std::cout << "Scanning..." << std::endl;
    
    bool isError = false;
	
    while ((!_isDone) && (!isError))
    {
        int length = 0;
	    
        unsigned char inputBuffer[HCI_MAX_EVENT_SIZE];
	    
        while ((!_isDone) && (_bluetoothControllerPtr->readDeviceInput(inputBuffer, HCI_MAX_EVENT_SIZE)) < 0)
        {            
            if (errno == EINTR)
            {
                _isDone = true;
	            
                break;
            }

            if ((errno == EAGAIN) || (errno == EINTR))
            {
                std::this_thread::sleep_for(std::chrono::microseconds(100));
	            
                continue;
            }

            isError = true;
        }

        if ((!_isDone) && (!isError))
        {     
            evt_le_meta_event* metaPtr = (evt_le_meta_event*)(inputBuffer + (1 + HCI_EVENT_HDR_SIZE));
            
            if (metaPtr->subevent != EVT_LE_ADVERTISING_REPORT)
            {
                continue;
            }
            
            uint8_t reportCount = metaPtr->data[0];
        
            void* offsetPtr = metaPtr->data + 1;
            
            while (reportCount--)
            {    
                le_advertising_info* infoPtr = (le_advertising_info*)(offsetPtr);
                
                uint8_t dataLength = infoPtr->length;

                if (dataLength == 0)
                {
                    continue;
                }
                
                // Manufacturer data is found from indices 2-4 (length: 3).
                
                unsigned char manufacturerData[4];
                
                manufacturerData[3] = 0;
                
                memcpy(manufacturerData, (infoPtr->data + 2), 3);
                
                std::string manufacturer = (char*)(manufacturerData);
                
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
                        
                        int8_t rssi = infoPtr->data[infoPtr->length];
                        
                        short temperature = getTemperature(payload.substr(0, 4));
                        unsigned short humidity = std::stoi(payload.substr(4, 3)) & 0xFFFF;
                        unsigned char battery = std::stoi(payload.substr(7, 2)) & 0xFF;
                        
                        if (_beacons.count(id) == 0)
                        {
                            BeaconState newBeacon;
                            
                            newBeacon.Temperature = temperature;
                            newBeacon.Humidity = humidity;
                            newBeacon.Battery = battery;
                            
                            _beacons.emplace(id, newBeacon);
                        }
                        
                        else
                        {
                            _beacons[id].Temperature = temperature;
                            _beacons[id].Humidity = humidity;
                            _beacons[id].Battery = battery;
                        }
                    }
                }
        
                offsetPtr = infoPtr->data + (infoPtr->length + 2);
            }
        }
    }
    
    _bluetoothControllerPtr->stopHCIScan_BLE();

    if (isError)
    {
        std::cerr << "Error scanning." << std::endl;
    }
}

void BaseController_RPi::finalise(void)
{
    _isDone = true;
}

short BaseController_RPi::getTemperature(std::string temperatureStr)
{
    short temperature = (std::stoi(temperatureStr.substr(1, 3)) & 0xFFFF);
    
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

std::string BaseController_RPi::base64Decode(unsigned char* inputBuffer, int inputLength)
{    
    std::string decodedString;
    
    unsigned char temp1[4];
    unsigned char temp2[3];
    
    int i = 0;
    
    int index = 0;
    
    // Handle data in sets of 4.

    while ((inputLength--) && (inputBuffer[index] != '=') && (isBase64(inputBuffer[index])))
    {       
        temp1[i++] = inputBuffer[index];
        
        ++index;
        
        if (i == 4)
        {      
            for (i = 0; i < 4; ++i)
            {
                temp1[i] = _base64Chars.find(temp1[i]);
            }

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