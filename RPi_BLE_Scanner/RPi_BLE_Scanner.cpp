﻿#include <exception>
#include <iostream>
#include <thread>

#include "BaseController_RPi.h"

//#define SERVERNAME "41.185.23.172"
//#define PORT 2226

//#define SERVERNAME "thermotrack.dyndns.org"
//#define PORT 2226

#define SERVERNAME "whizzdev.dyndns.org"
#define PORT "9062"

#define CONDUITNAME "RPi-Dev"

int main(int argc, char* argv[])
{
//    std::this_thread::sleep_for(std::chrono::minutes(1));
    
    std::string servername = SERVERNAME;
    std::string port = PORT;
    
    std::string conduitName = CONDUITNAME;
    
    std::string currentParam;
    
    if (argc > 1)
    {
        for (long int i = 1; i < argc; ++i)
        {
            try
            {
                currentParam = argv[i];
            }
            
            catch (...)
            {
                continue;
            }
            
            if (currentParam == "-s")
            {
                try
                {
                    servername = std::string(argv[i + 1]);
                }
                
                catch (...)
                {
                    servername = SERVERNAME;
                }
            }
            
            else if (currentParam == "-p")
            {             
                try
                {
                    port = std::string(argv[i + 1]);
                }
                
                catch (...)
                {
                    port = PORT;
                }
            }
            
            else if (currentParam == "-i")
            {             
                try
                {
                    conduitName = std::string(argv[i + 1]);
                }
                
                catch (...)
                {
                    conduitName = CONDUITNAME;
                }
            }
        }
    }
    
    BaseController_RPi* baseControllerPtr = nullptr;
    
    try
    {
        baseControllerPtr = new BaseController_RPi(servername, port, conduitName);
    }
    
    catch (const std::exception& e)
    {        
        if (baseControllerPtr != nullptr)
            delete baseControllerPtr;
        
        return 1;
    }

    std::thread listenerThread(&BaseController_RPi::listenForBLEDevices, baseControllerPtr);
    std::thread senderThread(&BaseController_RPi::sendDataPeriodically, baseControllerPtr);
    std::thread monitorThread(&BaseController_RPi::monitorSenderThread, baseControllerPtr);
    
    std::cout << "Enter 'q' (quit), 'e' (exit) or 'c' (close) to end the program..." << std::endl;
    
    std::string inputLine;
    
    std::getline(std::cin, inputLine);
    
    while ((!baseControllerPtr->getFinalised()) && (inputLine != "q") && (inputLine != "e") && (inputLine != "c"))
        std::getline(std::cin, inputLine);
    
    baseControllerPtr->setFinalised();

    monitorThread.join();
    senderThread.join();
    listenerThread.join();
    
    if (baseControllerPtr != nullptr)
        delete baseControllerPtr;

	return 0;
}