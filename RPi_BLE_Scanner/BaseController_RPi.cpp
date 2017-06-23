#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>

#include <sys/stat.h>

#include "BaseController_RPi.h"
#include "HTTPRequest_POST.h"

const std::string BaseController_RPi::_base64Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

BaseController_RPi::BaseController_RPi(const std::string servername, const std::string port, const std::string conduitName, const unsigned long int delaySenderLoopInSec):
    _servername(servername),
    _port(port),
    _conduitName(conduitName),
    _delaySenderLoopInSec(delaySenderLoopInSec)
{   
    try
    {
        _networkControllerPtr = new NetworkController_RPi();
    }
    
    catch (const std::exception& e)
    {
        logToFileWithSubdirectory(e, "Network");
        
        throw e;
    }
    
    try
    {
        _bluetoothControllerPtr = new BluetoothController();
    }
    
    catch (const std::exception& e)
    {
        logToFileWithSubdirectory(e, "Bluetooth");
        
        throw e;
    }
    
    try
    {
        _gsmControllerPtr = nullptr;
    }
    
    catch (const std::exception& e)
    {
        logToFileWithSubdirectory(e, "GSM");
        
        throw e;
    }
    
    _conduitNameLength = _conduitName.length();
    
    _isDone = false;
    
    _isReady = false;
    _isWaiting = false;
    _hasWoken = false;
    _beaconsCount = 0;
    _loopsCount = 0;
}

BaseController_RPi::~BaseController_RPi(void)
{
    std::map<std::string, PacketBLE*>::const_iterator it;
        
    for (it = _beacons.begin(); it != _beacons.end(); ++it)
        if (it->second != nullptr)
            delete it->second;
    
    if (_networkControllerPtr != nullptr)
        delete _networkControllerPtr;
    
    try
    {
        _bluetoothControllerPtr->stopHCIScan_BLE();
    }
    
    catch (const std::exception& e)
    {
        logToFileWithSubdirectory(e, "Bluetooth");
    }
    
    try
    {
        _bluetoothControllerPtr->closeHCIDevice();
    }
    
    catch (const std::exception& e)
    {
        logToFileWithSubdirectory(e, "Bluetooth");
    }
    
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
            std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
            
            if (_loopsCount > 0)
            {            
                // All client-listener threads wait for DELAY_SENDER_POST_IN_SEC seconds (interruptible).
                
                while (std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - start).count() < DELAY_SENDER_POST_IN_SEC)
                    if (_isDone)
                        break;
            }
            
            else
            {             
                // All client-listener threads wait for _delaySenderLoopInSec seconds (interruptible).
                
                while (std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - start).count() < _delaySenderLoopInSec)
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
    std::string baseID = "NULL";
    std::string baseLocation = "NULL";
    
    std::unique_lock<std::mutex> lock(_mutex);
    
    lock.unlock();
    
    while (!_isDone)
    {
        std::ifstream configFile("//home/pi/CONFIG/RPi_BLE_Scanner/config.json");
        
        if (configFile.is_open())
        {
            std::stringstream jsonStream;
                
            jsonStream << configFile.rdbuf();
                
            std::unordered_map<std::string, std::string> jsonPairs = getJSONPairs(jsonStream.str());
                
            std::size_t indexSpace;
                
            baseID = jsonPairs.at("BaseID");               
            baseLocation = jsonPairs.at("BaseLocation");
                
            configFile.close();
        }
        
        std::map<std::string, PacketBLE*>::const_iterator it;
        
        for (it = _beacons.begin(); it != _beacons.end(); ++it)
        {
            if (_isDone)
                return;
            
            time_t timeRaw;
            
            std::time(&timeRaw);
            
            struct tm timeInfo = *std::localtime(&timeRaw);
            
            char time[20];
            
            std::strftime(time, 20, "%F %T", &timeInfo);
            
            HTTPRequest_POST message("/api/blebeacons", _servername + ":" + _port, "close", "application/json");
            
            unsigned char buffer[HTTP_REQUEST_LENGTH_MAX];
            
            std::stringstream bodyStream;
            
            bodyStream << "{\"ID\": \"" << it->first << "\", \"Base64EncodedString\": \"" << it->second->payload << "\", \"RSSI\": " << it->second->rssi << ", \"Timestamp\": " << timeRaw << ", \"BaseID\": \"" << baseID << "\", \"BaseLocation\": \"" << baseLocation << "\"}";
            
            std::string body = bodyStream.str();
            
            unsigned long int bufferLength = body.length();
            
            std::memcpy(static_cast<void*>(buffer), static_cast<const void*>(body.c_str()), bufferLength);
            
            message.setContent(buffer, bufferLength);
            
            std::memset(static_cast<void*>(buffer), 0, bufferLength);
            
            bufferLength = message.serialise(buffer, sizeof(buffer));
            
            std::cout << "ID: " << it->first << " | Payload: " << it->second->payload << " | RSSI: " << it->second->rssi << " | Timestamp: " << time << std::endl;
            
            try
            {
                unsigned long int sessionID = _networkControllerPtr->connectToServer(_servername, _port);
                
                _networkControllerPtr->sendBufferWithSession(sessionID, buffer, bufferLength);
                
                std::this_thread::sleep_for(std::chrono::seconds(1));
                
                _networkControllerPtr->disconnectFromServer(sessionID);
            }
            
            catch (const std::exception& e)
            {
                logToFileWithSubdirectory(e, "Network");
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
        
        for (it = _beacons.begin(); it != _beacons.end(); ++it)
            if (it->second != nullptr)
                delete it->second;
        
        _beacons.clear();
        
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

void BaseController_RPi::listenForBLEDevices(void)
{
    try
    {
        _bluetoothControllerPtr->startHCIScan_BLE();
    }
    
    catch (const std::exception& e)
    {
        logToFileWithSubdirectory(e, "Bluetooth");
    }
	
    while (!_isDone)
    {
        unsigned char inputBuffer[HCI_MAX_EVENT_SIZE];
	    
        if ((_bluetoothControllerPtr->readDeviceInput(inputBuffer, HCI_MAX_EVENT_SIZE)) < 0)
        {
            if (errno == EAGAIN)
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            
            else
                _isDone = true;
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
                
                if (manufacturer == "INO")
                {
                    // RSSi is found at the index after the payload.
                    
                    long int rssi = static_cast<long int>(static_cast<signed char>(infoPtr->data[dataLength]));
                    
                    // Base64-encoded data is found from indices 10-22 (length: 13).
                    
                    unsigned char payloadData[13];
                    
                    std::memcpy(static_cast<void*>(payloadData), static_cast<const void*>(infoPtr->data + 10), 13);
                    
                    std::string payload(payloadData, payloadData + 13);

                    char idBuffer[6];
                    
                    ba2str(&infoPtr->bdaddr, idBuffer);
                    
                    std::string id = idBuffer;
                    
                    std::transform(id.begin(), id.end(), id.begin(), ::tolower);
                    
                    id.erase(std::remove(id.begin(), id.end(), ':'), id.end());
                    
                    if (_beacons.count(id) == 0)
                    {
                        _beacons.emplace(id, new PacketBLE(rssi, payload));
                        
                        ++_beaconsCount;
                    }
                        
                    else
                    {
                        _beacons.at(id)->rssi = rssi;
                        _beacons.at(id)->payload = payload;
                    }
                }
        
                offsetPtr = infoPtr->data + (infoPtr->length + 2);
            }
        }
    }
    
    _bluetoothControllerPtr->stopHCIScan_BLE();
}

void BaseController_RPi::setFinalised(void)
{
    _isDone = true;
}

bool BaseController_RPi::getFinalised(void)
{
    return _isDone;
}

short int BaseController_RPi::getTemperature(const std::string temperatureString)
{
    short int temperature = (std::stoi(temperatureString.substr(1, 3)) & 0xFFFF);
    
    if (temperatureString[0] == '-')
        temperature *= -1;
    
    return temperature;
}

std::unordered_map<std::string, std::string> BaseController_RPi::getJSONPairs(const std::string jsonString)
{
    std::unordered_map<std::string, std::string> jsonPairs;
    
    std::size_t indexBraceFirst, indexBraceLast;
    
    indexBraceFirst = jsonString.find_first_of('{');
    indexBraceLast = jsonString.find_last_of('}');
    
    std::string bodyString = jsonString.substr(indexBraceFirst + 1, indexBraceLast - (indexBraceFirst + 1));
    
    std::stringstream bodyStream(bodyString);
    
    std::string jsonPairCurrent;
    
    while (std::getline(bodyStream, jsonPairCurrent, ','))
    {
        std::size_t indexColon = jsonPairCurrent.find(':');
        
        std::string key = jsonPairCurrent.substr(0, indexColon);
        
        if (key.find('"') != std::string::npos)
        {
            std::size_t indexQuotationFirst, indexQuotationLast;
            
            indexQuotationFirst = key.find_first_of('"');
            indexQuotationLast = key.find_last_of('"');
            
            key = key.substr(indexQuotationFirst + 1, indexQuotationLast - (indexQuotationFirst + 1));
        }
        
        else
        {
            std::stringstream keyStream(key);
            
            keyStream >> key;
        }
        
        std::string value = jsonPairCurrent.substr(indexColon);
        
        if (value.find('"') != std::string::npos)
        {
            std::size_t indexQuotationFirst, indexQuotationLast;
            
            indexQuotationFirst = value.find_first_of('"');
            indexQuotationLast = value.find_last_of('"');
            
            value = value.substr(indexQuotationFirst + 1, indexQuotationLast - (indexQuotationFirst + 1));
        }
        
        else
        {
            std::stringstream valueStream(value);
            
            valueStream >> value;
        }
        
        jsonPairs.emplace(key, value);
    }
    
    return jsonPairs;
}

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

void BaseController_RPi::logToFileWithSubdirectory(const std::exception& e, std::string subdirectoryName)
{
    std::stringstream fileLogNameStream;
        
    fileLogNameStream << "//home/pi/LOGS/RPi_BLE_Scanner/" << subdirectoryName;
    
    umask(0);
    mkdir("//home/pi/LOGS", 0755);
    mkdir("//home/pi/LOGS/RPi_BLE_Scanner", 0755);
    mkdir(fileLogNameStream.str().c_str(), 0755);
    
    time_t timeRaw;
            
    std::time(&timeRaw);
            
    struct tm timeInfo = *std::localtime(&timeRaw);
            
    char time[20];
            
    std::strftime(time, 20, "%F_%T", &timeInfo);
        
    fileLogNameStream << "/" << time << ".log";
        
    std::ofstream fileLog(fileLogNameStream.str());
    
    if (fileLog.is_open())
        fileLog << e.what() << std::endl;
        
    fileLog.close();
}