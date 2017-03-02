#ifndef HTTPREQUEST_POST_H
#define HTTPREQUEST_POST_H

#include <cstring>
#include <sstream>

#include "HTTPRequest.h"

#define HTTP_REQUEST_POST_CONTENT_TYPE "application/json; charset=utf-8"

// Define the max HTTP POST content length as half of the the MTU (Maximum Transmission Unit).

#define HTTP_REQUEST_POST_CONTENT_LENGTH_MAX 700

class HTTPRequest_POST: public HTTPRequest
{
private:
    const std::string _contentType;    
    unsigned long int _contentLength;
    
    unsigned char _content[HTTP_REQUEST_POST_CONTENT_LENGTH_MAX];
    
public:
    HTTPRequest_POST(std::string requestURI): HTTPRequest("POST", requestURI), _contentType(HTTP_REQUEST_POST_CONTENT_TYPE), _contentLength(0) {}
    
    bool setContent(unsigned char* inputBuffer, const unsigned long int bufferLength)
    {
        if (bufferLength <= HTTP_REQUEST_POST_CONTENT_LENGTH_MAX)
        {
            _contentLength = bufferLength;
            
            std::memcpy(static_cast<void*>(_content), static_cast<void*>(inputBuffer), bufferLength);
            
            return true;
        }
        
        return false;
    }
    
    virtual unsigned long int serialise(unsigned char* outputBuffer, unsigned long int bufferLength)
    {
        std::stringstream outputStream;
        
        outputStream << _method << " " << _requestURI << " " << _versionHTTP << "\r\n";
        
        outputStream << "Content-Type: " << _contentType << "\r\n";
        
        outputStream << "Content-Length: " << _contentLength << "\r\n";
        
        outputStream << "\r\n";
        
        outputStream << _content;
        
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