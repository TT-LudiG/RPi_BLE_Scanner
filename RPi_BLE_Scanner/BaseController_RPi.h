#ifndef BASECONTROLLER_RPI_H
#define BASECONTROLLER_RPI_H

#include <atomic>
#include <map>

#include "BeaconState.h"
#include "BluetoothController.h"
#include "NetworkController_RPi.h"

class BaseController
{
private:
    NetworkController* _networkControllerPtr = nullptr;
    
    BluetoothController* _bluetoothControllerPtr = nullptr;
    
    std::atomic<bool> _isDone;
    
    std::map<std::string, BeaconState> _beacons;
    
    static const std::string _base64Chars;
    
    static short getTemperature(std::string temperatureStr);
    
    static bool isBase64(unsigned char inputChar);
    
    static std::string base64Decode(unsigned char* inputBuffer, int inputLength);
    
public:
    BaseController(void);
    BaseController(unsigned int port, std::string serverName);
    ~BaseController(void);
    
    void sendDataPeriodically(void);
    
    void listenforBLEDevices(void);
    
    void finalise(void);
};

#endif