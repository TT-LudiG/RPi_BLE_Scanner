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
    
    std::string getATResponse(const unsigned char* inputBuffer, const unsigned long int bufferLength);
    
    bool hasCorrectResponse(const std::string correctResponse);
    
public:
    GSMController(void);
    ~GSMController(void);
    
    void initialiseGPRS(void);
    
    void initialiseFTP(const std::string servername, const std::string username, const std::string password);
    
    void connectToServer(const std::string servername, const unsigned short int port);
    
    void connectToFTPServer(const std::string servername, const std::string username, const std::string password);
    
    void sendBuffer(const unsigned char* inputBuffer, const unsigned long int bufferLength);
    
//    void receiveBuffer(unsigned char* outputBuffer);
    
    void receiveFile(const std::string filename, const std::string filepath);
    
    void disconnectFromServer(void);
};
#endif