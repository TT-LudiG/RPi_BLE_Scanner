#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <string>

// Define the max HTTP request length as the MTU (Maximum Transmission Unit).

#define HTTP_REQUEST_LENGTH_MAX 1400

class HTTPRequest
{
protected:
    const std::string _method;
    const std::string _requestURI;
    const std::string _versionHTTP;
    
    const std::string _host;
    const std::string _connection;
    
public:
    HTTPRequest(const std::string method, const std::string requestURI, const std::string host, const std::string connection):
        _method(method),
        _requestURI(requestURI),
        _versionHTTP("HTTP/1.1"),
        _host(host),
        _connection(connection) {}
    
    virtual unsigned long int serialise(unsigned char* outputBuffer, const unsigned long int bufferLength) const = 0;
};

#endif