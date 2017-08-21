#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <string>

// Define the max HTTP request length as the MTU (Maximum Transmission Unit).

#define HTTP_REQUEST_LENGTH_MAX 1400

class HTTPRequest
{    
public:
    const std::string method;
    const std::string requestURI;
    const std::string versionHTTP;
    
    const std::string host;
    const std::string connection;
    
    HTTPRequest(const std::string method, const std::string requestURI, const std::string host, const std::string connection):
        method(method),
        requestURI(requestURI),
        versionHTTP("HTTP/1.1"),
        host(host),
        connection(connection) {}
    
    virtual unsigned long int serialise(unsigned char* outputBuffer, const unsigned long int bufferLength) const = 0;
};

#endif