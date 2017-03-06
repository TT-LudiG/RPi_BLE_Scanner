#ifndef NETWORKEXCEPTONS_H
#define NETWORKEXCEPTONS_H

#include <exception>

namespace NetworkExceptions
{
    // EXCEPTION_NET_0

    class WinSock2Exception: public std::exception
    {
    public:
        virtual const char* what() const throw()
        {
            return "EXCEPTION_NET_0: WinSock2 failed to initialise.";
        }
    };

    // EXCEPTION_NET_1

    class ServerAddressException: public std::exception
    {
    public:
        virtual const char* what() const throw()
        {
            return "EXCEPTION_NET_1: Failed to resolve the server address/port.";
        }
    };

    // EXCEPTION_NET_2

    class SocketGatewayException: public std::exception
    {
    public:
        virtual const char* what() const throw()
        {
            return "EXCEPTION_NET_2: Failed to create the gateway socket.";
        }
    };

    // EXCEPTION_NET_3

    class BlockingNonException: public std::exception
    {
    public:
        virtual const char* what() const throw()
        {
            return "EXCEPTION_NET_3: Failed to set the the gateway socket to non-blocking mode.";
        }
    };

    // EXCEPTION_NET_4

    class BindTCPException: public std::exception
    {
    public:
        virtual const char* what() const throw()
        {
            return "EXCEPTION_NET_4: Failed to set the the gateway socket as a TCP socket.";
        }
    };

    // EXCEPTION_NET_5

    class SocketListeningException: public std::exception
    {
    public:
        virtual const char* what() const throw()
        {
            return "EXCEPTION_NET_5: Gateway socket listening failed.";
        }
    };
}

#endif