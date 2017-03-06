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
    
public:
    HTTPRequest(std::string method, std::string requestURI, std::string host): _method(method), _requestURI(requestURI), _versionHTTP("HTTP/1.1"), _host(host) {}
    
    virtual unsigned long int serialise(unsigned char* outputBuffer, unsigned long int bufferLength) = 0;
};

#endif