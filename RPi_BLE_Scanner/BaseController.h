#ifndef BASECONTROLLER_H
#define BASECONTROLLER_H

#include <atomic>
#include <string>

#include "BluetoothController.h"

class BaseController
{
private:
    BluetoothController* _bluetoothControllerPtr;
    
    std::atomic<bool> _isDone;
    
    static const std::string _base64Chars;
    
    static bool isBase64(unsigned char inputChar);
    
    static std::string base64Decode(unsigned char* inputBuffer, int inputLength);
    
public:
    BaseController(void);    
    ~BaseController(void);
    
    void listenforBLEDevices(void);
    
    void finalise(void);
};

#endif