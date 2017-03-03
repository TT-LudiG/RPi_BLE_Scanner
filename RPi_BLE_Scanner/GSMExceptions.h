#ifndef GSMEXCEPTIONS_H
#define GSMEXCEPTIONS_H

#include <exception>

// EXCEPTION_GSM_0

class GSMBaseInitialiseException: public std::exception
{	
public:	
    virtual const char* what() const throw()
    {
        return "EXCEPTION_GSM_0: Failed to initialise the GSM controller.";
    }
};

// EXCEPTION_GSM_1

class GSMGPRSInitialiseException: public std::exception
{	
public:	
    virtual const char* what() const throw()
    {
        return "EXCEPTION_GSM_1: Failed to initialise GPRS.";
    }
};

// EXCEPTION_GSM_2

class GSMServerConnectException: public std::exception
{
private:
    const std::string _serverName;
    const unsigned short int _port;
	
public:
    GSMServerConnectException(const std::string serverName, const unsigned short int port): _serverName(serverName), _port(port) {}
	
    virtual const char* what() const throw()
    {
        std::string message = "EXCEPTION_GSM_2: Failed to connect to the server: " + _serverName + ", at port: " + std::to_string(_port) + ".";
		
        return message.c_str();
    }
};

// EXCEPTION_GSM_3

class GSMBufferSendException: public std::exception
{	
public:	
    virtual const char* what() const throw()
    {
        return "EXCEPTION_GSM_3: Failed to send data.";
    }
};

// EXCEPTION_GSM_4

class GSMBufferReceiveException: public std::exception
{	
public:	
    virtual const char* what() const throw()
    {
        return "EXCEPTION_GSM_4: Failed to receive data.";
    }
};

// EXCEPTION_GSM_5

class GSMServerDisconnectException: public std::exception
{	
public:	
    virtual const char* what() const throw()
    {
        return "EXCEPTION_GSM_5: Failed to disconnect from the server.";
    }
};

#endif