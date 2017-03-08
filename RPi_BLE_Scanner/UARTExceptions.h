#ifndef UARTEXCEPTIONS_H
#define UARTEXCEPTIONS_H

#include <exception>
#include <string>

namespace UARTExceptions
{
    // EXCEPTION_UART_0

    class TerminalOpenException: public std::exception
    {
    private:
        const std::string _error;
	
    public:
        TerminalOpenException(const std::string error): _error(error) {}
    
        virtual const char* what() const throw()
        {
            std::string message =  "EXCEPTION_UART_0: Failed to open the POSIX terminal file: " + _error;
        
            return message.c_str();
        }
    };

    // EXCEPTION_UART_1

    class TerminalAttributesGetException: public std::exception
    {
    private:
        const std::string _error;
	
    public:
        TerminalAttributesGetException(const std::string error): _error(error) {}
    
        virtual const char* what() const throw()
        {
            std::string message =  "EXCEPTION_UART_1: Failed to get the POSIX terminal attributes, with error: " + _error;
        
            return message.c_str();
        }
    };

    // EXCEPTION_UART_2

    class TerminalAttributesSetException: public std::exception
    {
    private:
        const std::string _error;
	
    public:
        TerminalAttributesSetException(const std::string error): _error(error) {}
    
        virtual const char* what() const throw()
        {
            std::string message =  "EXCEPTION_UART_2: Failed to set the POSIX terminal attributes, with error: " + _error;
        
            return message.c_str();
        }
    };

    // EXCEPTION_UART_3

    class TerminalWriteException: public std::exception
    {
    private:
        const std::string _error;
	
    public:
        TerminalWriteException(const std::string error): _error(error) {}
    
        virtual const char* what() const throw()
        {
            std::string message =  "EXCEPTION_UART_3: Failed to write to the POSIX terminal file, with error: " + _error;
        
            return message.c_str();
        }
    };

    // EXCEPTION_UART_4

    class TerminalReadException: public std::exception
    {
    private:
        const std::string _error;
	
    public:
        TerminalReadException(const std::string error): _error(error) {}
    
        virtual const char* what() const throw()
        {
            std::string message =  "EXCEPTION_UART_4: Failed to read from the POSIX terminal file, with error: " + _error;
        
            return message.c_str();
        }
    };
}

#endif