#ifndef NETWORKEXCEPTIONS_RPI_H
#define NETWORKEXCEPTIONS_RPI_H

#include <exception>
#include <string>

// EXCEPTION_NET_RPI_0

class SocketCreateException: public std::exception
{
private:
    const std::string _error;
	
public:
    SocketCreateException(const std::string error): _error(error) {}
	
    virtual const char* what() const throw()
    {
        std::string message = "EXCEPTION_NET_RPI_0: Failed to create the socket, with error: " + _error;
		
        return message.c_str();
    }
};

// EXCEPTION_NET_RPI_1

class HostnameLookupException: public std::exception
{
private:
    const std::string _hostname;
	
public:
    HostnameLookupException(const std::string hostname): _hostname(hostname) {}
	
    virtual const char* what() const throw()
    {
        std::string message = "EXCEPTION_NET_RPI_1: Hostname lookup failed: Unknown host " + _hostname;
		
        return message.c_str();
    }
};

// EXCEPTION_NET_RPI_2

class ServerConnectException: public std::exception
{
private:
    const std::string _error;
	
public:
    ServerConnectException(const std::string error): _error(error) {}
	
    virtual const char* what() const throw()
    {
        std::string message = "EXCEPTION_NET_RPI_2: Failed to connect to the server, with error: " + _error;
		
        return message.c_str();
    }
};

// EXCEPTION_NET_RPI_3

class ServerWriteException: public std::exception
{
private:
    const std::string _error;
	
public:
    ServerWriteException(const std::string error): _error(error) {}
	
    virtual const char* what() const throw()
    {
        std::string message = "EXCEPTION_NET_RPI_3: Failed to write to the server, with error: " + _error;
		
        return message.c_str();
    }
};

#endif