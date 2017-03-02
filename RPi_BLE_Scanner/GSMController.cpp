#include <chrono>
#include <sstream>
#include <thread>

#include "GSMController.h"
#include "GSMExceptions.h"

GSMController::GSMController(void)
{
	_uartControllerPtr = new UARTController("//dev/serial0");
    
    unsigned char command[4] = {"AT\r"};
    
    if (_uartControllerPtr->sendBuffer(command, 3) != 3)
    {       
        GSMInitialiseException e;
        throw e;
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    unsigned char response[255];
    
    int responseLength = _uartControllerPtr->receiveBuffer(response, sizeof(response));
    
    if (responseLength < 0)
    {
        GSMInitialiseException e;
        throw e;
    }
    
    if (getATResponse(response, responseLength) != "OK")
    {
        GSMInitialiseException e;
        throw e;
    }
}

GSMController::~GSMController(void) {}

int GSMController::sendBuffer(unsigned char* inputBuffer, const unsigned long int bufferLength)
{
    return 0;
}

int GSMController::receiveBuffer(unsigned char* outputBuffer)
{
    return 0;
}

std::string GSMController::getATResponse(unsigned char* inputBuffer, const unsigned long int bufferLength)
{
    std::stringstream responseStream;
    
    bool isResponse = false;
    
    for (int i = 0; i < bufferLength; ++i)
    {
        unsigned char currentChar = inputBuffer[i];
        
        if (isResponse)
        {        
            if ((currentChar != '\r') && (currentChar != '\n'))
                responseStream << currentChar;
        }
        
        else if ((currentChar == '\r') || (currentChar == '\n'))
            isResponse = true;
    }
    
    return responseStream.str();
}