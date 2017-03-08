#ifndef NETWORKEXCEPTONS_H
#define NETWORKEXCEPTONS_H

#include <exception>
#include <string>

namespace NetworkExceptions
{
    // EXCEPTION_NET_0

    class WinSock2Exception: public std::exception
    {
    public:
        virtual const char* what() const throw()
        {
            return "EXCEPTION_NET_0: Failed to initialise WinSock2.";
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
            return "EXCEPTION_NET_4: Failed to set the the gateway socket to TCP mode.";
        }
    };

    // EXCEPTION_NET_5

    class SocketListeningException: public std::exception
    {
    public:
        virtual const char* what() const throw()
        {
            return "EXCEPTION_NET_5: Failed to listen on the gateway socket.";
        }
    };

    // EXCEPTION_NET_6

    class GetNewSessionException: public std::exception
    {
    public:
        virtual const char* what() const throw()
        {
            return "EXCEPTION_NET_6: Failed to get a new TCP session.";
        }
    };
}

#endif