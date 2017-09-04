#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include <sstream>
#include <string>

// Define the max HTTP response length as the MTU (Maximum Transmission Unit).

#define HTTP_RESPONSE_LENGTH_MAX 1400

class HTTPResponse
{    
public:
    std::string versionHTTP;
    unsigned int statusCode;
    
    std::string contentType;
    unsigned long int contentLength;
    
    unsigned char content[HTTP_RESPONSE_LENGTH_MAX];
    
    HTTPResponse(void): statusCode(0), contentLength(0) {}
    
    bool deserialise(unsigned char* inputBuffer, const unsigned long int bufferLength)
    {
        if (bufferLength > HTTP_RESPONSE_LENGTH_MAX)
            return false;
        
        std::stringstream inputStream(std::string(inputBuffer, inputBuffer + bufferLength));
        
        std::string inputLine;
        
        std::getline(inputStream, inputLine);
        
        std::stringstream lineStream(inputLine);
        
        lineStream >> versionHTTP;
        lineStream >> statusCode;
        
        while (std::getline(inputStream, inputLine))
        {            
            if (inputLine == "\r")
            {
                // REINTERPRET_CAST!   
                
                inputStream.read(reinterpret_cast<char*>(content), contentLength);
                
                break;
            }
            
            std::stringstream lineStream(inputLine);
            
            std::string inputToken;
            
            lineStream >> inputToken;
            
            if (inputToken == "Content-Type:")
            {
                lineStream >> std::ws;
                
                std::getline(lineStream, contentType);
            }
            
            else if (inputToken == "Content-Length:")
                lineStream >> contentLength;
        }
        
        return true;
    }
};

#endif