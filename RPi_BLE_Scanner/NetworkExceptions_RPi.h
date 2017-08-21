#ifndef NETWORKEXCEPTIONS_RPI_H
#define NETWORKEXCEPTIONS_RPI_H

#include <exception>
#include <string>

namespace NetworkExceptions_RPi
{
    // EXCEPTION_NET_RPI_0

    class ServerLookupException: public std::exception
    {
    private:
        const std::string _servername;
        
        const std::string _error;
	
    public:
        ServerLookupException(const std::string servername, const std::string error):
            _servername(servername),
            _error(error) {}
	
        virtual const char* what() const throw()
        {
            std::string message = "EXCEPTION_NET_RPI_0: Lookup failed for servername: " + _servername + ", with error: " + _error;
		
            return message.c_str();
        }
    };
    
    // EXCEPTION_NET_RPI_1

    class SocketCreateException: public std::exception
    {
    private:
        const long int _socketHandle;
        
        const std::string _error;
	
    public:
        SocketCreateException(const long int socketHandle, const std::string error):
            _socketHandle(socketHandle),
            _error(error) {}
	
        virtual const char* what() const throw()
        {
            std::string message = "EXCEPTION_NET_RPI_1: Failed to create the socket: " + std::to_string(_socketHandle) + ", with error: " + _error;
		
            return message.c_str();
        }
    };
    
    // EXCEPTION_NET_RPI_2

    class SocketSetOptionException: public std::exception
    {
    private:
        const long int _socketHandle;
        
        const std::string _option;
        
        const std::string _error;
	
    public:
        SocketSetOptionException(const long int socketHandle, const std::string option, const std::string error):
            _socketHandle(socketHandle),
            _option(option),
            _error(error) {}
	
        virtual const char* what() const throw()
        {
            std::string message = "EXCEPTION_NET_RPI_2: Failed to set option: " + _option + ", for socket: " + std::to_string(_socketHandle) + ", with error: " + _error;
		
            return message.c_str();
        }
    };

    // EXCEPTION_NET_RPI_3

    class ServerConnectException: public std::exception
    {
    private:
        const std::string _servername;
        const std::string _port;
        
        const std::string _error;
	
    public:
        ServerConnectException(const std::string servername, const std::string port, const std::string error):
            _servername(servername),
            _port(port), 
            _error(error) {}
	
        virtual const char* what() const throw()
        {
            std::string message = "EXCEPTION_NET_RPI_3: Failed to connect to the server: " + _servername + ", at port: " + _port + ", with error: " + _error;
		
            return message.c_str();
        }
    };
    
    // EXCEPTION_NET_RPI_4

    class SocketWriteException: public std::exception
    {
    private:
        const long int _socketHandle;
        
        const std::string _error;
	
    public:
        SocketWriteException(const long int socketHandle, const std::string error):
            _socketHandle(socketHandle),
            _error(error) {}
	
        virtual const char* what() const throw()
        {
            std::string message = "EXCEPTION_NET_RPI_4: Failed to write to the socket: " + std::to_string(_socketHandle) + ", with error: " + _error;
		
            return message.c_str();
        }
    };

    // EXCEPTION_NET_RPI_5

    class SocketReadException: public std::exception
    {
    private:
        const long int _socketHandle;
        
        const std::string _error;
	
    public:
        SocketReadException(const long int socketHandle, const std::string error):
            _socketHandle(socketHandle),
            _error(error) {}
	
        virtual const char* what() const throw()
        {
            std::string message = "EXCEPTION_NET_RPI_5: Failed to read from the socket: " + std::to_string(_socketHandle) + ", with error: " + _error;
		
            return message.c_str();
        }
    };
}

#endif