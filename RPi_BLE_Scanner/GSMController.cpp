#include <chrono>
#include <cstring>
#include <sstream>
#include <thread>

#include "GSMController.h"
#include "GSMExceptions.h"

#define GSM_CONTROLLER_GET_RESPONSE_ATTEMPT_MAX 20

GSMController::GSMController(void)
{
	_uartControllerPtr = new UARTController("//dev/serial0");
    
    _isConnectedToServer = false;
    
    // AT Command: AT
    
    unsigned char command[] = {"AT\r"};
    
    if (_uartControllerPtr->sendBuffer(command, sizeof(command) - 1) < 0)
    {
        GSMExceptions::BaseInitialiseException e;
        throw e;
    }
    
    if (!hasCorrectResponse("ATOK"))
    {
        GSMExceptions::BaseInitialiseException e;
        throw e;
    }
}

GSMController::~GSMController(void)
{
    if (_isConnectedToServer)
        disconnectFromServer();
    
    if (_uartControllerPtr != nullptr)
        delete _uartControllerPtr;
}

void GSMController::initialiseGPRS(void)
{
    // AT Command: AT+CGATT?
    
    {    
        unsigned char command[] = {"AT+CGATT?\r"};
    
        if (_uartControllerPtr->sendBuffer(command, sizeof(command) - 1) < 0)
        {
            GSMExceptions::GPRSInitialiseException e;
            throw e;
        }
        
        if (!hasCorrectResponse("AT+CGATT?+CGATT: 1OK"))
        {
            GSMExceptions::GPRSInitialiseException e;
            throw e;
        }
    }
    
    // AT Command: AT+CIPSHUT
    
    {
        unsigned char command[] = {"AT+CIPSHUT\r"};
    
        if (_uartControllerPtr->sendBuffer(command, sizeof(command) - 1) < 0)
        {
            GSMExceptions::GPRSInitialiseException e;
            throw e;
        }
    
        if (!hasCorrectResponse("AT+CIPSHUTSHUT OK"))
        {
            GSMExceptions::GPRSInitialiseException e;
            throw e;
        }
    }
    
    // AT Command: AT+CIPMUX=0
    
    {
        unsigned char command[] = {"AT+CIPMUX=0\r"};
    
        if (_uartControllerPtr->sendBuffer(command, sizeof(command) - 1) < 0)
        {
            GSMExceptions::GPRSInitialiseException e;
            throw e;
        }
    
        if (!hasCorrectResponse("AT+CIPMUX=0OK"))
        {
            GSMExceptions::GPRSInitialiseException e;
            throw e;
        }
    }
    
    // AT Command: AT+CSTT="internet"
    
    {
        unsigned char command[] = {"AT+CSTT=\"internet\"\r"};
    
        if (_uartControllerPtr->sendBuffer(command, sizeof(command) - 1) < 0)
        {
            GSMExceptions::GPRSInitialiseException e;
            throw e;
        }
    
        if (!hasCorrectResponse("AT+CSTT=\"internet\"OK"))
        {
            GSMExceptions::GPRSInitialiseException e;
            throw e;
        }
    }
    
    // AT Command: AT+CIICR
    
    {  
        unsigned char command[] = {"AT+CIICR\r"};
    
        if (_uartControllerPtr->sendBuffer(command, sizeof(command) - 1) < 0)
        {
            GSMExceptions::GPRSInitialiseException e;
            throw e;
        }
        
        if (!hasCorrectResponse("AT+CIICROK"))
        {
            GSMExceptions::GPRSInitialiseException e;
            throw e;
        }
    }
    
    // AT Command: AT+CIFSR
    
    {
        unsigned char command[] = {"AT+CIFSR\r"};
    
        if (_uartControllerPtr->sendBuffer(command, sizeof(command) - 1) < 0)
        {
            GSMExceptions::GPRSInitialiseException e;
            throw e;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
        unsigned char response[255];
    
        long int responseLength = _uartControllerPtr->receiveBuffer(response, 255);
    
        if (responseLength < 0)
        {
            GSMExceptions::GPRSInitialiseException e;
            throw e;
        }
    
        _addressLocal = getATResponse(response, responseLength);
    }
}

void GSMController::connectToServer(const std::string servername, const unsigned short int port)
{
    // AT Command: AT+CIPSTART="TCP","serverName","port"
    
    std::stringstream commandStream;
    
    commandStream << "AT+CIPSTART=\"TCP\",\"" << servername << "\",\"" << port << "\"\r";
    
    std::string commandString = commandStream.str();
    
    unsigned long int commandLength = commandString.length();
    
    unsigned char command[commandLength];
    
    std::memcpy(static_cast<void*>(command), static_cast<const void*>(commandString.c_str()), commandLength);
    
    if (_uartControllerPtr->sendBuffer(command, commandLength) < 0)
    {
        GSMExceptions::ServerConnectException e(servername, port);
        throw e;
    }
    
    if (!hasCorrectResponse(commandString.substr(0, commandLength - 1) + "OKCONNECT OK"))
    {
        GSMExceptions::ServerConnectException e(servername, port);
        throw e;
    }
    
    _servername = servername;
    _port = port;
    
    _isConnectedToServer = true;
}

void GSMController::disconnectFromServer(void) const
{  
    // AT Command: AT+CIPCLOSE
    
    unsigned char command[] = {"AT+CIPCLOSE\r"};
    
    if (_uartControllerPtr->sendBuffer(command, sizeof(command) - 1) < 0)
    {
        GSMExceptions::ServerDisconnectException e;
        throw e;
    }
    
    if (!hasCorrectResponse("AT+CIPCLOSECLOSE OK"))
    {
        GSMExceptions::ServerDisconnectException e;
        throw e;
    }
}

void GSMController::sendBuffer(const unsigned char* inputBuffer, const unsigned long int bufferLength) const
{
    // AT Command: AT+CIPSEND
    
    {
        unsigned char command[] = {"AT+CIPSEND\r"};
    
        if (_uartControllerPtr->sendBuffer(command, sizeof(command) - 1) < 0)
        {
            GSMExceptions::BufferSendException e;
            throw e;
        }
        
        if (!hasCorrectResponse("AT+CIPSEND> "))
        {
            GSMExceptions::BufferSendException e;
            throw e;
        }
    
        unsigned char buffer[bufferLength + 1];
    
        std::memcpy(static_cast<void*>(buffer), static_cast<const void*>(inputBuffer), bufferLength);
    
        buffer[bufferLength] = 0x1A;
    
        if (_uartControllerPtr->sendBuffer(buffer, bufferLength + 1) < 0)
        {
            GSMExceptions::BufferSendException e;
            throw e;
        }
        
        std::string bufferString(buffer, buffer + bufferLength + 1);
        
        if (!hasCorrectResponse(bufferString + "SEND OK"))
        {
            GSMExceptions::BufferSendException e;
            throw e;
        }
    }
}

std::string GSMController::getATResponse(const unsigned char* inputBuffer, const unsigned long int bufferLength) const
{
    std::stringstream responseStream;
    
    for (long int i = 0; i < bufferLength; ++i)
    {
        unsigned char currentChar = inputBuffer[i];
        
        if ((currentChar != '\r') && (currentChar != '\n'))
            responseStream << currentChar;
    }
    
    return responseStream.str();
}

bool GSMController::hasCorrectResponse(const std::string correctResponse) const
{
    unsigned char response[255];
    
    std::memset(response, 0, 255);
    
    unsigned long int responseLength = 0;
    
    unsigned long int attempt = 0;
    
    while (getATResponse(response, responseLength) != correctResponse)
    {
        ++attempt;
        
        if (attempt > GSM_CONTROLLER_GET_RESPONSE_ATTEMPT_MAX)
            return false;
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        long int receiveLength = _uartControllerPtr->receiveBuffer(response + responseLength, 255);
    
        if (receiveLength < 0)
            return false;
        
        responseLength += receiveLength;
    }
    
    return true;
}