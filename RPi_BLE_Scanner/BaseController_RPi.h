#ifndef BASECONTROLLER_RPI_H
#define BASECONTROLLER_RPI_H

#include <atomic>
#include <map>

#include "BeaconState.h"
#include "BluetoothController.h"
#include "NetworkController_RPi.h"

class BaseController_RPi
{
private:
    NetworkController_RPi* _networkControllerPtr = nullptr;
    
    BluetoothController* _bluetoothControllerPtr = nullptr;
    
    std::atomic<bool> _isDone;
    
    std::map<std::string, BeaconState> _beacons;
    
    static const std::string _base64Chars;
    
    static short getTemperature(std::string temperatureStr);
    
    static bool isBase64(unsigned char inputChar);
    
    static std::string base64Decode(unsigned char* inputBuffer, int inputLength);
    
public:
    BaseController_RPi(void);
    BaseController_RPi(std::string serverName, unsigned int port);
    
    ~BaseController_RPi(void);
    
    void sendDataPeriodically(void);
    
    void listenforBLEDevices(void);
    
    void finalise(void);
};

#endif