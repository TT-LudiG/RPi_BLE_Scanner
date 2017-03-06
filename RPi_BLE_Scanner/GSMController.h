#ifndef GSMCONTROLLER_H
#define GSMCONTROLLER_H

#include "UARTController.h"

class GSMController
{
private:
    UARTController* _uartControllerPtr;
    
    std::string _addressLocal;
    
    std::string _serverName;
    unsigned short int _port;
    
    bool _isConnectedToServer;
    
    std::string getATResponse(unsigned char* inputBuffer, const unsigned long int bufferLength);
    
    bool hasCorrectResponse(std::string correctResponse);
    
public:
    GSMController(void);
    ~GSMController(void);
    
    void initialiseGPRS(void);
    
    void connectToServer(std::string serverName, unsigned short int port);
    
    void sendBuffer(const unsigned char* inputBuffer, const unsigned long int bufferLength);
    
//    void receiveBuffer(unsigned char* outputBuffer);
    
    void disconnectFromServer(void);
};
#endif