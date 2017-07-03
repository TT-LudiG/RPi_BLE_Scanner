#include <exception>
#include <iostream>
#include <thread>

#include "BaseController_RPi.h"

#define SERVERNAME "41.185.23.172"
#define PORT "49997"

#define CONDUITNAME "RPi-Dev"

#define DELAY_SENDER_LOOP_IN_SEC 300;

int main(int argc, char* argv[])
{  
    std::string servername = SERVERNAME;
    std::string port = PORT;
    
    std::string conduitName = CONDUITNAME;
    
    unsigned long int delaySenderLoopInSec = DELAY_SENDER_LOOP_IN_SEC;
    
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
            
            else if (currentParam == "-t")
            {             
                try
                {
                    delaySenderLoopInSec = std::stoul(std::string(argv[i + 1]));
                }
                
                catch (...)
                {
                    delaySenderLoopInSec = DELAY_SENDER_LOOP_IN_SEC;
                }
            }
        }
    }
    
    BaseController_RPi* baseControllerPtr = nullptr;
    
    try
    {
        baseControllerPtr = new BaseController_RPi(servername, port, conduitName, delaySenderLoopInSec);
    }
    
    catch (const std::exception& e)
    {        
        if (baseControllerPtr != nullptr)
            delete baseControllerPtr;
        
        return 1;
    }

    std::thread listenerThread(&BaseController_RPi::listenForBLEDevices, baseControllerPtr);
    
    std::thread senderThread(&BaseController_RPi::sendDataPeriodically, baseControllerPtr);
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
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