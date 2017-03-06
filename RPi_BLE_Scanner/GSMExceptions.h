#ifndef GSMEXCEPTIONS_H
#define GSMEXCEPTIONS_H

#include <exception>

namespace GSMExceptions
{
    // EXCEPTION_GSM_0

    class BaseInitialiseException: public std::exception
    {	
    public:	
        virtual const char* what() const throw()
        {
            return "EXCEPTION_GSM_0: Failed to initialise the GSM controller.";
        }
    };

    // EXCEPTION_GSM_1

    class GPRSInitialiseException: public std::exception
    {	
    public:	
        virtual const char* what() const throw()
        {
            return "EXCEPTION_GSM_1: Failed to initialise GPRS.";
        }
    };

    // EXCEPTION_GSM_2

    class ServerConnectException: public std::exception
    {
    private:
        const std::string _serverName;
        const unsigned short int _port;
	
    public:
        ServerConnectException(const std::string serverName, const unsigned short int port): _serverName(serverName), _port(port) {}
	
        virtual const char* what() const throw()
        {
            std::string message = "EXCEPTION_GSM_2: Failed to connect to the server: " + _serverName + ", at port: " + std::to_string(_port) + ".";
		
            return message.c_str();
        }
    };

    // EXCEPTION_GSM_3

    class BufferSendException: public std::exception
    {	
    public:	
        virtual const char* what() const throw()
        {
            return "EXCEPTION_GSM_3: Failed to send data.";
        }
    };

    // EXCEPTION_GSM_4

    class BufferReceiveException: public std::exception
    {	
    public:	
        virtual const char* what() const throw()
        {
            return "EXCEPTION_GSM_4: Failed to receive data.";
        }
    };

    // EXCEPTION_GSM_5

    class ServerDisconnectException: public std::exception
    {	
    public:	
        virtual const char* what() const throw()
        {
            return "EXCEPTION_GSM_5: Failed to disconnect from the server.";
        }
    };
}

#endif