#ifndef GSMCONTROLLER_H
#define GSMCONTROLLER_H

#include "UARTController.h"

class GSMController
{
private:
    UARTController* _uartControllerPtr;
    
    std::string _addressLocal;
    
    std::string _servername;
    unsigned short int _port;
    
    bool _isConnectedToServer;
    
    std::string getATResponse(const unsigned char* inputBuffer, const unsigned long int bufferLength) const;
    
    bool hasCorrectResponse(const std::string correctResponse) const;
    
public:
    GSMController(void);
    ~GSMController(void);
    
    void initialiseGPRS(void);
    
    void connectToServer(const std::string servername, const unsigned short int port);    
    void disconnectFromServer(void) const;
    
    void sendBuffer(const unsigned char* inputBuffer, const unsigned long int bufferLength) const;
};
#endif