#include <cerrno>
#include <cstring>

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include "UARTController.h"
#include "UARTExceptions.h"

UARTController::UARTController(std::string ttyDevice)
{  
    _uartFileHandle = open(ttyDevice.c_str(), O_RDWR | O_NOCTTY);
    
    if (_uartFileHandle < 0)
    {
        UARTExceptions::TerminalOpenException e(std::string(std::strerror(errno)));
        throw e;
    }
    
    struct termios options;
    
    std::memset(&options, 0, sizeof(options));
    
    if (tcgetattr(_uartFileHandle, &options) < 0)
    {
        UARTExceptions::TerminalAttributesGetException e(std::string(std::strerror(errno)));
        throw e;
    }
    
    // Set the POSIX terminal baud rate.
    
    cfsetispeed(&options, B9600);
    cfsetospeed(&options, B9600);
    
    // Disable the POSIX terminal's parity checks.
    
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    
    // Set POSIX terminal's char size to 8 bits.
    
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    
    // Set POSIX terminal as local and enable reading from the terminal.
    
    options.c_cflag |= CLOCAL;
    options.c_cflag |= CREAD;
    
    // Set POSIX terminal output mode to raw.
    
    options.c_oflag &= ~OPOST;
    
    // Set the POSIX terminal to non-canonical mode, disable echoing and disable interrupts.

    options.c_lflag &= ~ICANON;
    options.c_lflag &= ~ECHO;
    options.c_lflag &= ~ECHOE;
    options.c_lflag &= ~ISIG;
    
    // Set the read timeout to 0.1 seconds, and the minimum chars-to-read count to 0.
    
    options.c_cc[VTIME] = 1;
    options.c_cc[VMIN] = 0;
    
    if (tcsetattr(_uartFileHandle, TCSAFLUSH, &options) < 0)
    {
        UARTExceptions::TerminalAttributesSetException e(std::string(std::strerror(errno)));
        throw e;
    }
}

UARTController::~UARTController(void)
{   
    close(_uartFileHandle);
}
    
int UARTController::sendBuffer(const unsigned char* inputBuffer, const unsigned long int bufferLength)
{
    int bytesCount = -1;
    
    if (_uartFileHandle != -1)
    {
        try
        {
            bytesCount = write(_uartFileHandle, static_cast<const void*>(inputBuffer), bufferLength);
        }
        
        catch (const std::exception&)
        {
            UARTExceptions::TerminalWriteException e(std::string(std::strerror(errno)));
            throw e;
        }
    }
    
    return bytesCount;
}

int UARTController::receiveBuffer(unsigned char* outputBuffer, const unsigned long int bufferLength)
{
    int bytesCount = -1;

    if (_uartFileHandle != -1)
    {
        try
        {
            bytesCount = read(_uartFileHandle, static_cast<void*>(outputBuffer), bufferLength);
        }
        
        catch (const std::exception&)
        {
            UARTExceptions::TerminalReadException e(std::string(std::strerror(errno)));
            throw e;
        }
    }

    return bytesCount;
}