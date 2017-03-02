#ifndef UARTEXCEPTIONS_H
#define UARTEXCEPTIONS_H

#include <exception>

// EXCEPTION_UART_0

class UARTOpenException: public std::exception
{
private:
    const std::string _error;
	
public:
    UARTOpenException(const std::string error): _error(error) {}
    
    virtual const char* what() const throw()
    {
        std::string message =  "EXCEPTION_UART_0: Failed to open the POSIX terminal file: " + _error;
        
        return message.c_str();
    }
};

// EXCEPTION_UART_1

class UARTAttributesGetException: public std::exception
{
private:
    const std::string _error;
	
public:
    UARTAttributesGetException(const std::string error): _error(error) {}
    
    virtual const char* what() const throw()
    {
        std::string message =  "EXCEPTION_UART_1: Failed to get the POSIX terminal attributes, with error: " + _error;
        
        return message.c_str();
    }
};

// EXCEPTION_UART_2

class UARTAttributesSetException: public std::exception
{
private:
    const std::string _error;
	
public:
    UARTAttributesSetException(const std::string error): _error(error) {}
    
    virtual const char* what() const throw()
    {
        std::string message =  "EXCEPTION_UART_2: Failed to set the POSIX terminal attributes, with error: " + _error;
        
        return message.c_str();
    }
};

// EXCEPTION_UART_3

class UARTWriteException: public std::exception
{
private:
    const std::string _error;
	
public:
    UARTWriteException(const std::string error): _error(error) {}
    
    virtual const char* what() const throw()
    {
        std::string message =  "EXCEPTION_UART_3: Failed to write to the POSIX terminal file, with error: " + _error;
        
        return message.c_str();
    }
};

// EXCEPTION_UART_4

class UARTReadException: public std::exception
{
private:
    const std::string _error;
	
public:
    UARTReadException(const std::string error): _error(error) {}
    
    virtual const char* what() const throw()
    {
        std::string message =  "EXCEPTION_UART_4: Failed to read from the POSIX terminal file, with error: " + _error;
        
        return message.c_str();
    }
};

#endif