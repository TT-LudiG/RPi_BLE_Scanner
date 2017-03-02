#ifndef BASECONTROLLER_RPI_H
#define BASECONTROLLER_RPI_H

#include <atomic>
#include <condition_variable>
#include <map>
#include <mutex>

#include "BeaconState.h"
#include "BluetoothController.h"
#include "GSMController.h"
#include "NetworkController_RPi.h"

class BaseController_RPi
{
private:
    NetworkController_RPi* _networkControllerPtr = nullptr;
    
    BluetoothController* _bluetoothControllerPtr = nullptr;
    
    GSMController* _gsmControllerPtr = nullptr;

    std::atomic<bool> _isDone;

    std::mutex _mutex;
    std::atomic<bool> _isReady;
    std::atomic<bool> _isWaiting;
    std::atomic<bool> _hasWoken;
    std::atomic<unsigned int> _beaconsCount;
    std::atomic<unsigned int> _loopsCount;

    std::condition_variable _cv;
    
    std::map<std::string, BeaconState*> _beacons;
    
    static const std::string _base64Chars;
    
    static short getTemperature(const std::string& temperatureStr);
    
    static bool isBase64(const unsigned char inputChar);
    
    static std::string base64Decode(const unsigned char* inputBuffer, const unsigned long int bufferLength);
    
public:
    BaseController_RPi(void);
    BaseController_RPi(const std::string serverName, const unsigned int port);
    
    ~BaseController_RPi(void);
    
    void monitorSenderThread(void);
    
    void sendDataPeriodically(void);
    
    void listenforBLEDevices(void);
    
    void finalise(void);
};

#endif