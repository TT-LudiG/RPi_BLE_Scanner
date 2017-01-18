#ifndef BASECONTROLLER_RPI_H
#define BASECONTROLLER_RPI_H

#include <atomic>
#include <bitset>
#include <string>

#include "BluetoothController.h"
#include "NetworkController_RPi.h"

class BaseController
{
private:
    NetworkController* _networkController;
    
    BluetoothController* _bluetoothControllerPtr;
    
    std::atomic<bool> _isDone;
    
    static const std::string _base64Chars;
    
    static std::bitset<10> getTemperatureBits(std::string temperatureStr);    
    static std::bitset<9> getHumidityBits(std::string ambientStr);
    static std::bitset<7> getBatteryBits(std::string batteryStr);
    
    static bool isBase64(unsigned char inputChar);
    
    static std::string base64Decode(unsigned char* inputBuffer, int inputLength);
    
public:
    BaseController(void);
    BaseController(unsigned int port, std::string serverName);
    ~BaseController(void);
    
    void listenforBLEDevices(void);
    
    void finalise(void);
};

#endif