#ifndef UARTCONTROLLER_H
#define UARTCONTROLLER_H

#include <string>

class UARTController
{
private:
    int _uartFileHandle;
    
public:
    UARTController(std::string ttyDevice);
    ~UARTController(void);
    
    void sendBuffer(unsigned char* inputBuffer, const unsigned long int bufferLength);
    
    int receiveBuffer(unsigned char* outputBuffer);
};
#endif