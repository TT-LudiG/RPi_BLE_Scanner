#include <algorithm>
#include <cctype>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include "BaseController_RPi.h"
#include "HTTPRequest_GET.h"
#include "HTTPRequest_POST.h"

// Constructor.

BaseController_RPi::BaseController_RPi(const std::string servername_general, const std::string servername_data, const std::string port_general, const std::string port_data, const unsigned long int delay_sender_loop_in_sec):
    _servername_general(servername_general),
    _servername_data(servername_data),
    _port_general(port_general),
    _port_data(port_data),
    _delay_sender_loop_in_sec(delay_sender_loop_in_sec),
    _networkControllerPtr(nullptr),
    _bluetoothControllerPtr(nullptr)
{
    try
    {
        _networkControllerPtr = new NetworkController_RPi();
    }
    
    catch (const std::exception& e)
    {
        logToFileWithSubdirectory(e.what(), "Network");
        
        throw e;
    }
    
    try
    {
        _bluetoothControllerPtr = new BluetoothController();
    }
    
    catch (const std::exception& e)
    {
        logToFileWithSubdirectory(e.what(), "Bluetooth");
        
        throw e;
    }
    
    _isDone = false;
    _isScanning = false;
    
    _isReady = false;
    _isWaiting = false;
    _hasWoken = false;
    _loopsCount = 0;
}

// Destructor.

BaseController_RPi::~BaseController_RPi(void)
{
    std::map<std::string, PacketBLE*>::const_iterator it;
        
    for (it = _beacons.begin(); it != _beacons.end(); ++it)
        if (it->second != nullptr)
            delete it->second;
    
    if (_networkControllerPtr != nullptr)
        delete _networkControllerPtr;
    
    if (_isScanning)
    {
        try
        {
            _bluetoothControllerPtr->stopHCIScan_BLE();
        }
    
        catch (const std::exception& e)
        {
            logToFileWithSubdirectory(e.what(), "Bluetooth");
        }
    }
    
    if (_bluetoothControllerPtr != nullptr)
        delete _bluetoothControllerPtr;
}

// Method to manage the timing of the sender thread by locking its mutex.

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
                
                while (std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - start).count() < _delay_sender_loop_in_sec)
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

// Method to send HTTP POSTs of the BLE beacon data (JSON format) to the Web API.

void BaseController_RPi::sendDataPeriodically(void)
{    
    std::unique_lock<std::mutex> lock(_mutex);
    
    lock.unlock();
    
    // First attempt to get the ID.
    
    _id = getID();
    
    while (!_isDone)
    {
        // Subsequent attempts to get the ID, if not yet found.
        
        if (_id == 0)
            _id = getID();
        
        else
        {        
            // Send an alive status notification to the ThermoTrack_API_BLE_General Web API.
            
            std::stringstream requestURIStream;
            requestURIStream << "/api/blereaders/" << _id;
            
            sendPOSTToServerURI(_servername_general, _port_general, requestURIStream.str(), "", 2000);
            
            // Send all recently received BLE packets to the ThermoTrack_API_BLE_Temperature Web API.
            
            std::map<std::string, PacketBLE*>::const_iterator it;
            
            for (it = _beacons.begin(); it != _beacons.end(); ++it)
            {
                if (_isDone)
                    return;
                
                std::string timestamp = getTimeString_Now("%F %T");
                
                std::cout << "ID: " << it->first << " | Value: " << it->second->Value << " | Battery: " << it->second->Battery << " | Timestamp: " << timestamp << std::endl;
            
                std::stringstream bodyStream;
                bodyStream << "{\"BoltIdentifier\":\"" << it->first << "\", \"Battery\":" << it->second->Battery << ", \"Value\":" << it->second->Value << ", \"HostID\":\"0\", \"DateTime\":\"" << timestamp << "\"}";               
                sendPOSTToServerURI(_servername_data, _port_data, "/api/blebeacons", bodyStream.str(), 2000);
                
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

// Method to listen for BLE beacon broadcasts.

void BaseController_RPi::listenForBLEDevices(void)
{
    try
    {
        _bluetoothControllerPtr->startHCIScan_BLE();
        
        _isScanning = true;
    }
    
    catch (const std::exception& e)
    {
        logToFileWithSubdirectory(e.what(), "Bluetooth");
    }
	
    while (!_isDone)
    {      
        unsigned char inputBuffer[HCI_MAX_EVENT_SIZE];
        
        long int inputLength = _bluetoothControllerPtr->readDeviceInput(inputBuffer, HCI_MAX_EVENT_SIZE);
	    
        if (inputLength < 0)
        {
            if (errno == EAGAIN)
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            
            else
                _isDone = true;
        }

        else if (inputLength > 0)
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
                
                std::string manufacturer;
                std::string payload;
                
                bool isBreak = false;
                
                std::stringstream dataStream;
                
                std::string token = "";
                
                for (unsigned long int i = 0; i < dataLength; ++i)
                {
                    unsigned char byte = infoPtr->data[i];
                    
                    if (std::isprint(byte))
                    {
                        token += byte;
                        
                        isBreak = true;
                    }
                    
                    else if (isBreak)
                    {
                        if (token.length() == 3)
                            manufacturer = token;
                        
                        else
                            payload = token;
                        
                        token = "";
                        
                        isBreak = false;
                    }
                }
                
                if (manufacturer == "GMS")
                {
                    std::string payloadDecoded = base64Decode(payload.substr(1));
                    
                    char idBuffer[6];
                        
                    ba2str(&infoPtr->bdaddr, idBuffer);
                        
                    std::string id = idBuffer;
                    
                    id.erase(std::remove(id.begin(), id.end(), ':'), id.end());
                        
                    float value = stof(payloadDecoded.substr(0, 5));
                    float battery = stof(payloadDecoded.substr(5, 5));
                        
                    if (_beacons.count(id) == 0)
                    {
                        _beacons.emplace(id, new PacketBLE(value, battery));
                    }
                        
                    else
                    {
                        _beacons.at(id)->Value = value;
                        _beacons.at(id)->Battery = battery;
                    }
                }
        
                offsetPtr = infoPtr->data + (infoPtr->length + 2);
            }
        }
    }
    
    try
    {
        _bluetoothControllerPtr->stopHCIScan_BLE();
        
        _isScanning = false;
    }
    
    catch (const std::exception& e)
    {
        logToFileWithSubdirectory(e.what(), "Bluetooth");
    }
}

void BaseController_RPi::setFinalised(void)
{
    _isDone = true;
}

bool BaseController_RPi::getFinalised(void) const
{
    return _isDone;
}

// Method to get the reader ID.

unsigned long int BaseController_RPi::getID(void) const
{
    unsigned long int id = 0;
    
    // Check the config file for an existing ReaderID.
    
    if (fileExists("//home/pi/CONFIG", "RPi_BLE_Scanner.config"))
    {
        std::ifstream fileConfig("//home/pi/CONFIG/RPi_BLE_Scanner.config");
        
        if (fileConfig.is_open())
        {
            std::stringstream jsonStream;
                
            jsonStream << fileConfig.rdbuf();
                
            std::unordered_map<std::string, std::string> jsonPairs = getJSONPairs(jsonStream.str());
            
            id = std::stoul(jsonPairs.at("ReaderID"));
            
            fileConfig.close();
            
            return id;
        }
    }
    
    // Send a ReaderID request to the ThermoTrack_API_BLE_General Web API.
    
    std::string uri = "/api/blereaders/" + getMACAddress();
    
    HTTPResponse response = sendGETToServerURI(_servername_general, _port_general, uri, 2000);
    
    // Write the ReaderID response to the config file.
    
    if ((response.statusCode == 200) && (response.contentLength > 0))
    {
        id = std::stoul(std::string(response.content, response.content + response.contentLength));
        
        std::ofstream fileConfig("//home/pi/CONFIG/RPi_BLE_Scanner.config");
        
        fileConfig << "{ ReaderID: " << id << " }";
    }
    
    return id;
}

// Method to send an HTTP GET message via the NetworkController.

HTTPResponse BaseController_RPi::sendGETToServerURI(const std::string servername, const std::string port, const std::string uri, const unsigned long int responseWaitInMs) const
{
    HTTPResponse response;
    
    unsigned char responseBuffer[HTTP_REQUEST_LENGTH_MAX];
    
    long int responseBufferLength = -1;
    
    HTTPRequest_GET message(uri, servername + ":" + port, "close", "application/json");
            
    unsigned char buffer[HTTP_REQUEST_LENGTH_MAX];
            
    unsigned long int bufferLength = message.serialise(buffer, sizeof(buffer));
    
    try
    {
        unsigned long int sessionID = _networkControllerPtr->connectToServer(servername, port);
                
        _networkControllerPtr->sendBufferWithSession(sessionID, buffer, bufferLength);
                
        std::this_thread::sleep_for(std::chrono::milliseconds(responseWaitInMs));
        
        responseBufferLength = _networkControllerPtr->receiveBufferWithSession(sessionID, responseBuffer, HTTP_REQUEST_LENGTH_MAX);
                
        _networkControllerPtr->disconnectFromServer(sessionID);
    }
            
    catch (const std::exception& e)
    {
        logToFileWithSubdirectory(e.what(), "Network");
    }
    
    if (responseBufferLength > 0)
        response.deserialise(responseBuffer, responseBufferLength);
    
    return response;
}

// Method to send an HTTP POST message via the NetworkController.

HTTPResponse BaseController_RPi::sendPOSTToServerURI(const std::string servername, const std::string port, const std::string uri, const std::string body, const unsigned long int responseWaitInMs) const
{
    HTTPResponse response;
    
    unsigned char responseBuffer[HTTP_REQUEST_LENGTH_MAX];
    
    long int responseBufferLength = -1;
    
    HTTPRequest_POST message(uri, servername + ":" + port, "close", "application/json");
    
    unsigned char buffer[HTTP_REQUEST_LENGTH_MAX];
    
    unsigned long int bufferLength = body.length();
    
    std::memcpy(static_cast<void*>(buffer), static_cast<const void*>(body.c_str()), bufferLength);
    
    message.setContent(buffer, bufferLength);
    
    std::memset(static_cast<void*>(buffer), 0, bufferLength);
    
    bufferLength = message.serialise(buffer, sizeof(buffer));
    
    try
    {
        unsigned long int sessionID = _networkControllerPtr->connectToServer(servername, port);
                
        _networkControllerPtr->sendBufferWithSession(sessionID, buffer, bufferLength);
                
        std::this_thread::sleep_for(std::chrono::milliseconds(responseWaitInMs));
        
        responseBufferLength = _networkControllerPtr->receiveBufferWithSession(sessionID, responseBuffer, HTTP_REQUEST_LENGTH_MAX);
                
        _networkControllerPtr->disconnectFromServer(sessionID);
    }
            
    catch (const std::exception& e)
    {
        logToFileWithSubdirectory(e.what(), "Network");
    }
    
    if (responseBufferLength > 0)
        response.deserialise(responseBuffer, responseBufferLength);
    
    return response;
}

// Method to check if a file exists locally.

bool BaseController_RPi::fileExists(const std::string directoryPath, const std::string fileName)
{
    bool fileExists = false;
    
    DIR* directoryPtr = opendir(directoryPath.c_str());
    
    if (directoryPtr != nullptr)
    {      
        struct dirent* directoryEntryPtr = readdir(directoryPtr);
        
        while (directoryEntryPtr != nullptr)
        {
            std::string fileNamTemp = std::string(directoryEntryPtr->d_name);
            
            if (fileNamTemp == fileName)
            {
                fileExists = true;
                
                break;
            }
            
            directoryEntryPtr = readdir(directoryPtr);
        }
        
        closedir(directoryPtr);
    }
    
    return fileExists;
}

// Method to log an message to file.

void BaseController_RPi::logToFileWithSubdirectory(const std::string message, const std::string subdirectoryName)
{
    std::stringstream fileLogNameStream;
    
    fileLogNameStream << "//home/pi/LOGS/RPi_BLE_Scanner/" << subdirectoryName;
    
    umask(0);
    mkdir("//home/pi/LOGS", 0755);
    mkdir("//home/pi/LOGS/RPi_BLE_Scanner", 0755);
    mkdir(fileLogNameStream.str().c_str(), 0755);
    
    fileLogNameStream << "/" << getTimeString_Now("%F_%T") << ".log";
    
    std::ofstream fileLog(fileLogNameStream.str());
    
    if (fileLog.is_open())
        fileLog << message << std::endl;
    
    fileLog.close();
}

// Method to get a temperature value from a string.

short int BaseController_RPi::getTemperature(const std::string temperatureString)
{
    short int temperature = (std::stoi(temperatureString.substr(1, 3)) & 0xFFFF);
    
    if (temperatureString[0] == '-')
        temperature *= -1;
    
    return temperature;
}

// Method to get JSON pairs from a JSON string.

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
        
        std::string value = jsonPairCurrent.substr(indexColon + 1);
        
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

// Method to get the local device's MAC address.

std::string BaseController_RPi::getMACAddress(void)
{
    std::string addressMAC = "";
    
    std::ifstream fileMACAddress("//sys/class/net/eth0/address");
    
    if (fileMACAddress.is_open())
        fileMACAddress >> addressMAC;
    
    fileMACAddress.close();
    
    std::transform(addressMAC.begin(), addressMAC.end(), addressMAC.begin(),::tolower);
    
    addressMAC.erase(std::remove(addressMAC.begin(), addressMAC.end(), ':'), addressMAC.end());
    
    return addressMAC;
}

// Method to get current time (raw).

unsigned long int BaseController_RPi::getTimeRaw_Now(void)
{   
    time_t timeRaw;
            
    std::time(&timeRaw);
            
    struct tm timeInfo = *std::localtime(&timeRaw);
    
    return timeRaw;
}

// Method to get current time (string).

std::string BaseController_RPi::getTimeString_Now(const std::string format)
{
    std::string timeString = "";
    
    time_t timeRaw;
            
    std::time(&timeRaw);
            
    struct tm timeInfo = *std::localtime(&timeRaw);
            
    char timeBuffer[20];
            
    std::strftime(timeBuffer, 20, format.c_str(), &timeInfo);
    
    timeString = std::string(timeBuffer);
    
    return timeString;
}

// The "base64Decode" function is based off of 3rd-party source code (since altered), distributed under the following license:

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

std::string BaseController_RPi::base64Decode(const std::string inputString)
{
    std::string decodedString;
    
    const std::string base64Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    
    unsigned char temp1[4];
    unsigned char temp2[3];
    
    unsigned char i = 0;
    
    unsigned long int index = 0;
    
    // Handle data in sets of 4.
    
    unsigned long int currentLength = inputString.length();
    
    char currentChar = inputString.at(index);

    while ((currentLength--) && ((isalnum(currentChar)) || (currentChar == '+') || (currentChar == '/')))
    {       
        temp1[i++] = inputString.at(index);
        
        if (i == 4)
        {      
            for (i = 0; i < 4; ++i)
                temp1[i] = base64Chars.find(temp1[i]);

            temp2[0] = (temp1[0] << 2) + ((temp1[1] & 0x30) >> 4);
            temp2[1] = ((temp1[1] & 0xf) << 4) + ((temp1[2] & 0x3c) >> 2);
            temp2[2] = ((temp1[2] & 0x3) << 6) + temp1[3];

            for (i = 0; i < 3; ++i)      
                decodedString += temp2[i];
        
            i = 0;
        }
        
        currentChar = inputString.at(++index);
    }
    
    // Handle an incomplete final set (>4).

    if (i > 0)
    {
        for (unsigned char j = i; j < 4; ++j)
            temp1[j] = 0;

        for (unsigned char j = 0; j < 4; ++j)
            temp1[j] = base64Chars.find(temp1[j]);
        
        temp2[0] = (temp1[0] << 2) + ((temp1[1] & 0x30) >> 4);
        temp2[1] = ((temp1[1] & 0xf) << 4) + ((temp1[2] & 0x3c) >> 2);
        temp2[2] = ((temp1[2] & 0x3) << 6) + temp1[3];

        for (unsigned char j = 0; j < (i - 1); ++j)           
            decodedString += temp2[j];
    }

    return decodedString;
}