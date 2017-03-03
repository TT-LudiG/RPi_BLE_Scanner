#ifndef HTTPREQUEST_GET_H
#define HTTPREQUEST_GET_H

#include <cstring>
#include <sstream>

#include "HTTPRequest.h"

class HTTPRequest_GET: public HTTPRequest
{    
public:
    HTTPRequest_GET(std::string requestURI, std::string host): HTTPRequest("GET", requestURI, host) {}
    
    virtual unsigned long int serialise(unsigned char* outputBuffer, unsigned long int bufferLength)
    {
        std::stringstream outputStream;
        
        outputStream << _method << " " << _requestURI << " " << _versionHTTP << "\r\n";
        
        outputStream << "Host: " << _host << "\r\n";
        
        outputStream << "\r\n";
        
        std::string outputString = outputStream.str();
        
        unsigned long int outputLength = outputString.length();
        
        if (outputLength <= bufferLength)
        {     
            std::memcpy(static_cast<void*>(outputBuffer), static_cast<const void*>(outputString.c_str()), outputLength);
            
            return outputLength;
        }
        
        return 0;
    }
};

#endif