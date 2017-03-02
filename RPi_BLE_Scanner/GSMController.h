#ifndef GSMCONTROLLER_H
#define GSMCONTROLLER_H

#include "UARTController.h"

class GSMController
{
private:
    UARTController* _uartControllerPtr;
    
    std::string getATResponse(unsigned char* inputBuffer, const unsigned long int bufferLength);
    
public:
    GSMController(void);
    ~GSMController(void);
    
    int sendBuffer(unsigned char* inputBuffer, const unsigned long int bufferLength);
    
    int receiveBuffer(unsigned char* outputBuffer);
};
#endif