#include <algorithm>
#include <chrono>
#include <iostream>
#include <thread>

#include <iomanip>
#include <sstream>

#include "BaseController_RPi.h"
#include "HTTPRequest_POST.h"

const std::string BaseController_RPi::_base64Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

BaseController_RPi::BaseController_RPi(const std::string servername, const unsigned short int port): _servername(servername), _port(port)
{
    _networkControllerPtr = new NetworkController_RPi();
    _bluetoothControllerPtr = new BluetoothController();  
    _gsmControllerPtr = nullptr;
    
    _isDone = false;
    
    _isReady = false;
    _isWaiting = false;
    _hasWoken = false;
    _beaconsCount = 0;
    _loopsCount = 0;
}

BaseController_RPi::~BaseController_RPi(void)
{
    std::map<std::string, BeaconState*>::const_iterator it;
    
    for (it = _beacons.begin(); it != _beacons.end(); ++it)
        if (it->second != nullptr)
            delete it->second;
    
    if (_networkControllerPtr != nullptr)
        delete _networkControllerPtr;
    
    _bluetoothControllerPtr->stopHCIScan_BLE();
    
    _bluetoothControllerPtr->closeHCIDevice();
    
    if (_bluetoothControllerPtr != nullptr)
        delete _bluetoothControllerPtr;
    
    if (_gsmControllerPtr != nullptr)
        delete _gsmControllerPtr;
}

void BaseController_RPi::monitorSenderThread(void)
{
    std::unique_lock<std::mutex> lock(_mutex);

    while (!_isDone)
    {
        // Reset the conditional wait barrier for all client-listener threads.

        if ((_isReady) && (_hasWoken))
        {
            lock.lock();

            _isReady = false;
            
            _hasWoken = false;
        }

        if ((!_isReady) && (_isWaiting))
        {
            std::clock_t start = std::clock();
            
            if (_loopsCount > 0)
            {
                // All client-listener threads wait for 1 seconds (interruptible).

                while (((std::clock() - start) / static_cast<double>(CLOCKS_PER_SEC)) < 1.0)
                    if (_isDone)
                        break;
            }
            
            else
            {
                // All client-listener threads wait for 5 seconds (interruptible).

                while (((std::clock() - start) / static_cast<double>(CLOCKS_PER_SEC)) < 5.0)
                    if (_isDone)
                        break;
            }

            _isReady = true;

            lock.unlock();

            _cv.notify_all();

            _isWaiting = false;
        }
    }
    
    _isReady = true;
}

void BaseController_RPi::sendDataPeriodically(void)
{
    std::unique_lock<std::mutex> lock(_mutex);
    
    lock.unlock();
    
    while (!_isDone)
    {
        std::map<std::string, BeaconState*>::const_iterator it;
        
        for (it = _beacons.begin(); it != _beacons.end(); ++it)
        {
            if (_isDone)
                return;
            
            HTTPRequest_POST message("/api/v1/SetRecord", "127.0.0.1");
            
            time_t timeRaw;
            
            std::time(&timeRaw);
            
            struct tm timeInfo = *std::localtime(&timeRaw);
            
            char time[20];
            
            std::strftime(time, 20, "%F %T", &timeInfo);
            
            std::stringstream contentStream;
    
            contentStream << "{\"BoltIdentifier\":\"" << it->first << "\", \"Battery\":" << it->second->Battery << ", \"Value\":" << it->second->Value << ", \"HostID\":\"0\", \"DateTime\":\"" << time << "\"}";
    
            std::string contentString = contentStream.str();
    
            unsigned long int contentLength = contentString.length();
    
            unsigned char content[contentLength];
    
            std::memcpy(static_cast<void*>(content), static_cast<const void*>(contentString.c_str()), contentLength);
    
            message.setContent(content, sizeof(content));
    
            unsigned char buffer[HTTP_REQUEST_LENGTH_MAX];
    
            unsigned long int bufferLength = message.serialise(buffer, sizeof(buffer));
            
            try
            {              
                unsigned long int sessionID = _networkControllerPtr->connectToServer(_servername, _port);
                
                _networkControllerPtr->sendBufferWithSession(sessionID, buffer, bufferLength);
                
                _networkControllerPtr->disconnectFromServer(sessionID);
            }
            
            catch (const std::exception& e)
            {
                std::cerr << e.what() << std::endl;
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            
            ++_loopsCount;
            
            _isWaiting = true;
            
            lock.lock();

            while (!_isReady)
                _cv.wait(lock);
            
            _hasWoken = true;
            
            lock.unlock();
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        
        _loopsCount = 0;
        
        _isWaiting = true;
            
        lock.lock();

        while (!_isReady)
            _cv.wait(lock);
            
        _hasWoken = true;
        
        lock.unlock();
    }
}

void BaseController_RPi::listenforBLEDevices(void)
{	
    _bluetoothControllerPtr->startHCIScan_BLE();
	
    while (!_isDone)
    {
        unsigned char inputBuffer[HCI_MAX_EVENT_SIZE];
	    
        if ((_bluetoothControllerPtr->readDeviceInput(inputBuffer, HCI_MAX_EVENT_SIZE)) < 0)
        {
            if (errno == EAGAIN)
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            
            else
            {
                if (errno == EINTR)
                    std::cerr << "Interrupted system call." << std::endl;
                
                std::cerr << "Scanning error." << std::endl;
                
                break;
            }
        }

        else
        {
            // REINTERPRET_CAST!
            
            evt_le_meta_event* metaPtr = reinterpret_cast<evt_le_meta_event*>(inputBuffer + (1 + HCI_EVENT_HDR_SIZE));
            
            if (metaPtr->subevent != EVT_LE_ADVERTISING_REPORT)
                continue;
            
            unsigned char reportCount = metaPtr->data[0];
            
            void* offsetPtr = metaPtr->data + 1;
            
            while (reportCount--)
            {    
                le_advertising_info* infoPtr = static_cast<le_advertising_info*>(offsetPtr);
                
                unsigned char dataLength = infoPtr->length;

                if (dataLength == 0)
                    continue;
                
                // Manufacturer data is found from indices 2-4 (length: 3).
                
                unsigned char manufacturerData[3];
                
                std::memcpy(static_cast<void*>(manufacturerData), static_cast<const void*>(infoPtr->data + 2), 3);
                
                std::string manufacturer(manufacturerData, manufacturerData + 3);
                
                if (manufacturer == "GMS")
                {                   
                    // Base64-encoded data is found from indices 10-24 (length: 15).
                    
                    unsigned char payloadData[15];
                    
                    std::memcpy(static_cast<void*>(payloadData), static_cast<const void*>(infoPtr->data + 11), 15);
                    
                    std::string payload = base64Decode(payloadData, 15);
                    
                    if (payload.size() == 10)
                    {
                        char idBuffer[6];
                        
                        ba2str(&infoPtr->bdaddr, idBuffer);
                        
                        std::string id = idBuffer;
                        
                        id.erase(std::remove(id.begin(), id.end(), ':'), id.end());
                        
                        float value = stof(payload.substr(0, 5));
                        float battery = stof(payload.substr(5, 5));
                        
//                        short int temperature = getTemperature(payload.substr(0, 4));
//                        unsigned short int humidity = std::stoi(payload.substr(4, 3)) & 0xFFFF;
//                        unsigned char battery = std::stoi(payload.substr(7, 2)) & 0xFF;
                        
                        if (_beacons.count(id) == 0)
                        {
                            _beacons.emplace(id, new BeaconState(value, battery));
                            
                            ++_beaconsCount;
                        }
                        
                        else
                        {
                            _beacons.at(id)->Value = value;
                            _beacons.at(id)->Battery = battery;
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

//short int BaseController_RPi::getTemperature(const std::string temperatureString)
//{
//    short int temperature = (std::stoi(temperatureString.substr(1, 3)) & 0xFFFF);
//    
//    if (temperatureString[0] == '-')
//        temperature *= -1;
//    
//    return temperature;
//}

// The "isBase64" and "base64Decode" functions were based off of 3rd-party source code (since altered), distributed under the following license:

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

bool BaseController_RPi::isBase64(const unsigned char inputChar)
{
    return (isalnum(inputChar) || (inputChar == '+') || (inputChar == '/'));
}

std::string BaseController_RPi::base64Decode(const unsigned char* inputBuffer, const unsigned long int bufferLength)
{    
    std::string decodedString;
    
    unsigned char temp1[4];
    unsigned char temp2[3];
    
    unsigned char i = 0;
    
    unsigned long int index = 0;
    
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
        for (unsigned char j = i; j < 4; ++j)
            temp1[j] = 0;

        for (unsigned char j = 0; j < 4; ++j)
            temp1[j] = _base64Chars.find(temp1[j]);
        
        temp2[0] = (temp1[0] << 2) + ((temp1[1] & 0x30) >> 4);
        temp2[1] = ((temp1[1] & 0xf) << 4) + ((temp1[2] & 0x3c) >> 2);
        temp2[2] = ((temp1[2] & 0x3) << 6) + temp1[3];

        for (unsigned char j = 0; j < (i - 1); ++j)           
            decodedString += temp2[j];
    }

    return decodedString;
}