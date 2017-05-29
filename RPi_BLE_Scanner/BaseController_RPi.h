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

#define DELAY_SENDER_POST_IN_SEC 1
#define DELAY_SENDER_LOOP_IN_SEC 5

class BaseController_RPi
{
private:
    NetworkController_RPi* _networkControllerPtr;
    BluetoothController* _bluetoothControllerPtr;
    GSMController* _gsmControllerPtr;
    
    const std::string _servername;
    const std::string _port;
    
    const std::string _conduitName;
    unsigned long int _conduitNameLength;

    std::atomic<bool> _isDone;

    std::mutex _mutex;
    std::atomic<bool> _isReady;
    std::atomic<bool> _isWaiting;
    std::atomic<bool> _hasWoken;
    std::atomic<unsigned long int> _beaconsCount;
    std::atomic<unsigned long int> _loopsCount;

    std::condition_variable _cv;
    
    std::map<std::string, BeaconState*> _beacons;
    
    static const std::string _base64Chars;
    
    static short int getTemperature(const std::string temperatureString);
    
    static bool isBase64(const unsigned char inputChar);
    static std::string base64Decode(const unsigned char* inputBuffer, const unsigned long int bufferLength);
    
    static void logToFileWithSubdirectory(const std::exception& e, std::string subdirectoryName);
    
public:
    BaseController_RPi(const std::string servername, const std::string port, const std::string conduitName);
    ~BaseController_RPi(void);
    
    void monitorSenderThread(void);
    
    void sendDataPeriodically(void);
    
    void listenForBLEDevices(void);
    
    void setFinalised(void);
    bool getFinalised(void);
};

#endif