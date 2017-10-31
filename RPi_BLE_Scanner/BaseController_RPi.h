#ifndef BASECONTROLLER_RPI_H
#define BASECONTROLLER_RPI_H

#include <atomic>
#include <condition_variable>
#include <map>
#include <mutex>

#include "BluetoothController.h"
#include "GSMController.h"
#include "HTTPResponse.h"
#include "NetworkController_RPi.h"
#include "PacketBLE.h"

#define DELAY_SENDER_POST_IN_SEC 1

class BaseController_RPi
{
private:
    NetworkController_RPi* _networkControllerPtr;
    BluetoothController* _bluetoothControllerPtr;
    
    const std::string _servername_general;
    const std::string _servername_data;
    const std::string _port_general;
    const std::string _port_data;
    const unsigned long int _delay_sender_loop_in_sec;
    
    unsigned long int _id;

    std::atomic<bool> _isDone;
    std::atomic<bool> _isScanning;

    std::mutex _mutex;
    std::atomic<bool> _isReady;
    std::atomic<bool> _isWaiting;
    std::atomic<bool> _hasWoken;
    std::atomic<unsigned long int> _loopsCount;

    std::condition_variable _cv;
    
    std::map<std::string, PacketBLE*> _beacons;
    
    unsigned long int getID(void) const;
    
    HTTPResponse sendGETToServerURI(const std::string servername, const std::string port, const std::string uri, const unsigned long int responseWaitInMs) const;
    HTTPResponse sendPOSTToServerURI(const std::string servername, const std::string port, const std::string uri, const std::string body, const unsigned long int responseWaitInMs) const;
    
    static bool fileExists(const std::string directoryName, const std::string fileName);
    
    static void logToFileWithSubdirectory(const std::string message, const std::string subdirectoryName);
    
    static short int getTemperature(const std::string temperatureString);
    static std::unordered_map<std::string, std::string> getJSONPairs(const std::string jsonString);    
    static std::string getMACAddress(void);
    static unsigned long int getTimeRaw_Now(void);
    static std::string getTimeString_Now(const std::string format);
    
    static std::string base64Decode(const unsigned char* inputBuffer, const unsigned long int bufferLength);
    
public:
    BaseController_RPi(const std::string servername_general, const std::string servername_data, const std::string port_general, const std::string port_data, const unsigned long int delay_sender_loop_in_sec);
    ~BaseController_RPi(void);
    
    void monitorSenderThread(void);
    
    void sendDataPeriodically(void);
    
    void listenForBLEDevices(void);
    
    void setFinalised(void);
    bool getFinalised(void) const;
};

#endif