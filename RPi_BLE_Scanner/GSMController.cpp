#include <chrono>
#include <cstring>
#include <sstream>
#include <thread>

#include "GSMController.h"
#include "GSMExceptions.h"

GSMController::GSMController(void)
{
    // AT Command: AT
    
	_uartControllerPtr = new UARTController("//dev/serial0");
    
    unsigned char command[] = {"AT\r"};
    
    if (_uartControllerPtr->sendBuffer(command, sizeof(command) - 1) < 0)
    {
        GSMBaseInitialiseException e;
        throw e;
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    unsigned char response[255];
    
    int responseLength = _uartControllerPtr->receiveBuffer(response, sizeof(response));
    
    if (responseLength < 0)
    {
        GSMBaseInitialiseException e;
        throw e;
    }
    
    if (getATResponse(response, responseLength) != "OK")
    {
        GSMBaseInitialiseException e;
        throw e;
    }
}

GSMController::~GSMController(void)
{
    disconnectFromServer();
}

void GSMController::initialiseGPRS(void)
{
    // AT Command: AT+CGATT?
    
    {    
        unsigned char command[] = {"AT+CGATT?\r"};
    
        if (_uartControllerPtr->sendBuffer(command, sizeof(command) - 1) < 0)
        {
            GSMGPRSInitialiseException e;
            throw e;
        }
    
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
        unsigned char response[255];
    
        int responseLength = _uartControllerPtr->receiveBuffer(response, sizeof(response));
    
        if (responseLength < 0)
        {
            GSMGPRSInitialiseException e;
            throw e;
        }
    
        if (getATResponse(response, responseLength) != "+CGATT:1")
        {
            GSMGPRSInitialiseException e;
            throw e;
        }
    }
    
    // AT Command: AT+CIPSHUT
    
    {
        unsigned char command[] = {"AT+CIPSHUT\r"};
    
        if (_uartControllerPtr->sendBuffer(command, sizeof(command) - 1) < 0)
        {
            GSMGPRSInitialiseException e;
            throw e;
        }
    
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
        unsigned char response[255];
    
        unsigned char responseLength = _uartControllerPtr->receiveBuffer(response, sizeof(response));
    
        if (responseLength < 0)
        {
            GSMGPRSInitialiseException e;
            throw e;
        }
    
        if (getATResponse(response, responseLength) != "SHUT OK")
        {
            GSMGPRSInitialiseException e;
            throw e;
        }
    }
    
    // AT Command: AT+CIPMUX=0
    
    {
        unsigned char command[] = {"AT+CIPMUX=0\r"};
    
        if (_uartControllerPtr->sendBuffer(command, sizeof(command) - 1) < 0)
        {
            GSMGPRSInitialiseException e;
            throw e;
        }
    
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
        unsigned char response[255];
    
        unsigned char responseLength = _uartControllerPtr->receiveBuffer(response, sizeof(response));
    
        if (responseLength < 0)
        {
            GSMGPRSInitialiseException e;
            throw e;
        }
    
        if (getATResponse(response, responseLength) != "OK")
        {
            GSMGPRSInitialiseException e;
            throw e;
        }
    }
    
    // AT Command: AT+CSTT="internet"
    
    {
        unsigned char command[] = {"AT+CSTT=\"internet\"\r"};
    
        if (_uartControllerPtr->sendBuffer(command, sizeof(command) - 1) < 0)
        {
            GSMGPRSInitialiseException e;
            throw e;
        }
    
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
        unsigned char response[255];
    
        unsigned char responseLength = _uartControllerPtr->receiveBuffer(response, sizeof(response));
    
        if (responseLength < 0)
        {
            GSMGPRSInitialiseException e;
            throw e;
        }
    
        if (getATResponse(response, responseLength) != "OK")
        {
            GSMGPRSInitialiseException e;
            throw e;
        }
    }
    
    // AT Command: AT+CIICR
    
    {  
        unsigned char command[] = {"AT+CIICR\r"};
    
        if (_uartControllerPtr->sendBuffer(command, sizeof(command) - 1) < 0)
        {
            GSMGPRSInitialiseException e;
            throw e;
        }
    
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
        unsigned char response[255];
    
        unsigned char responseLength = _uartControllerPtr->receiveBuffer(response, sizeof(response));
    
        if (responseLength < 0)
        {
            GSMGPRSInitialiseException e;
            throw e;
        }
    
        if (getATResponse(response, responseLength) != "OK")
        {
            GSMGPRSInitialiseException e;
            throw e;
        }
    }
    
    // AT Command: AT+CIFSR
    
    {
        unsigned char command[] = {"AT+CIFSR\r"};
    
        if (_uartControllerPtr->sendBuffer(command, sizeof(command) - 1) < 0)
        {
            GSMGPRSInitialiseException e;
            throw e;
        }
    
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
        unsigned char response[255];
    
        unsigned char responseLength = _uartControllerPtr->receiveBuffer(response, sizeof(response));
    
        if (responseLength < 0)
        {
            GSMGPRSInitialiseException e;
            throw e;
        }
    
        _addressLocal = getATResponse(response, responseLength);
    }
}

void GSMController::connectToServer(std::string serverName, unsigned short int port)
{
    // AT Command: AT+CIPSTART="TCP","serverName","port"
    
    std::stringstream commandStream;
    
    commandStream << "AT+CIPSTART=\"TCP\",\"" << serverName << "\",\"" << port << "\"\r";
    
    std::string commandString = commandStream.str();
    
    unsigned long int commandLength = commandString.length();
    
    unsigned char command[commandLength];
    
    std::memcpy(static_cast<void*>(command), static_cast<const void*>(commandString.c_str()), commandLength);
    
    if (_uartControllerPtr->sendBuffer(command, commandLength) < 0)
    {
        GSMServerConnectException e(serverName, port);
        throw e;
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    unsigned char response[255];
    
    unsigned char responseLength = _uartControllerPtr->receiveBuffer(response, sizeof(response));
    
    if (responseLength < 0)
    {
        GSMServerConnectException e(serverName, port);
        throw e;
    }
    
    if (getATResponse(response, responseLength) != "Connected")
    {
        GSMServerConnectException e(serverName, port);
        throw e;
    }
}

void GSMController::sendBuffer(const unsigned char* inputBuffer, const unsigned long int bufferLength)
{
    // AT Command: AT+CIPSEND
    
    {
        unsigned char command[] = {"AT+CIPSEND\r"};
    
        if (_uartControllerPtr->sendBuffer(command, sizeof(command) - 1) < 0)
        {
            GSMBufferSendException e;
            throw e;
        }
    
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
        unsigned char response[255];
    
        unsigned char responseLength = _uartControllerPtr->receiveBuffer(response, sizeof(response));
    
        if (responseLength < 0)
        {
            GSMBufferSendException e;
            throw e;
        }
    
        if (getATResponse(response, responseLength) != ">")
        {
            GSMBufferSendException e;
            throw e;
        }
    
        unsigned char buffer[bufferLength + 1];
    
        std::memcpy(static_cast<void*>(buffer), static_cast<const void*>(inputBuffer), bufferLength);
    
        buffer[bufferLength] = 0x1A;
    
        if (_uartControllerPtr->sendBuffer(buffer, bufferLength + 1) < 0)
        {
            GSMBufferSendException e;
            throw e;
        }
    }
    
    // AT Command: AT+CIPSHUT
    
    {
        unsigned char command[] = {"AT+CIPSHUT\r"};
    
        if (_uartControllerPtr->sendBuffer(command, sizeof(command) - 1) < 0)
        {
            GSMBufferSendException e;
            throw e;
        }
    
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
        unsigned char response[255];
    
        unsigned char responseLength = _uartControllerPtr->receiveBuffer(response, sizeof(response));
    
        if (responseLength < 0)
        {
            GSMBufferSendException e;
            throw e;
        }
    
        if (getATResponse(response, responseLength) != "SHUT OK")
        {
            GSMBufferSendException e;
            throw e;
        }
    }
}

void GSMController::receiveBuffer(unsigned char* outputBuffer) {}

void GSMController::disconnectFromServer(void)
{
    // AT Command: AT+CIPCLOSE
    
    unsigned char command[] = {"AT+CIPCLOSE\r"};
    
    if (_uartControllerPtr->sendBuffer(command, sizeof(command) - 1) < 0)
    {
        GSMServerDisconnectException e;
        throw e;
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    unsigned char response[255];
    
    unsigned char responseLength = _uartControllerPtr->receiveBuffer(response, sizeof(response));
    
    if (responseLength < 0)
    {
        GSMServerDisconnectException e;
        throw e;
    }
    
    if (getATResponse(response, responseLength) != "OK")
    {
        GSMServerDisconnectException e;
        throw e;
    }
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