#include <chrono>
#include <iostream>
#include <thread>

#include "BaseController.h"

const std::string BaseController::_base64Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

BaseController::BaseController(void)
{
    _bluetoothControllerPtr = new BluetoothController();
    
    _isDone = false;
}

BaseController::~BaseController(void)
{
    _bluetoothControllerPtr->stopHCIScan_BLE();
    
    _bluetoothControllerPtr->closeHCIDevice();
    
    delete _bluetoothControllerPtr;
}

void BaseController::listenforBLEDevices(void)
{	
    _bluetoothControllerPtr->startHCIScan_BLE();
    
    std::cout << "Scanning..." << std::endl;
	
    std::string inputLine;
    
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
            
                char addressBuffer[18];
        
                ba2str(&(infoPtr->bdaddr), addressBuffer);
                
                std::string address = addressBuffer;
                
                int rssi = int(infoPtr->data[dataLength]);
                
                // Manufacturer data is found from indices 2-4 (length: 3).
                
                unsigned char manufacturerData[4];
                
                manufacturerData[3] = 0;
                
                memcpy(manufacturerData, (infoPtr->data + 2), 3);
                
                std::string manufacturer = (char*)(manufacturerData);
                
                // Temperature data is found from indices 10-22 (length: 13).
                
                unsigned char payloadData[12];
                
                memcpy(payloadData, (infoPtr->data + 11), 12);
                
                std::string payload = base64Decode(payloadData, 12);
                
                std::string temperature = "NULL";
                std::string humidity = "NULL";
                std::string battery = "NULL";
                
                if (payload.size() == 9)
                {                
                    temperature = payload.substr(0, 4);
                    humidity = payload.substr(4, 3);
                    battery = payload.substr(7, 2);
                }
                
                std::cout << address << " | RSSI: " << rssi << " | Manufacturer: "  << manufacturer << " | Temperature: " << temperature << " | Humidity: " << humidity << " | Battery: " << battery << std::endl;
        
                offsetPtr = infoPtr->data + (infoPtr->length + 2);
            }
        }
    }
    
    _bluetoothControllerPtr->stopHCIScan_BLE();

    if (isError)
    {
        std::cout << "Error scanning." << std::endl;
    }
}

void BaseController::finalise(void)
{
    _isDone = true;
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

bool BaseController::isBase64(unsigned char inputChar)
{
    return (isalnum(inputChar) || (inputChar == '+') || (inputChar == '/'));
}

std::string BaseController::base64Decode(unsigned char* inputBuffer, int inputLength)
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