#ifndef UARTCONTROLLER_H
#define UARTCONTROLLER_H

#include <string>

class UARTController
{
private:
    int _uartFileHandle;
    
public:
    UARTController(const std::string ttyDevice);
    ~UARTController(void);
    
    long int sendBuffer(const unsigned char* inputBuffer, const unsigned long int bufferLength) const;   
    long int receiveBuffer(unsigned char* outputBuffer, const unsigned long int bufferLength) const;
};
#endif