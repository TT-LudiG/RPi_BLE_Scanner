#include <exception>
#include <iostream>
#include <thread>

#include "BaseController_RPi.h"

//#define SERVERNAME "thermotrack.dyndns.org"
//#define PORT 2226

#define SERVERNAME "intellibolt.dyndns.org"
#define PORT 8062

int main(int argc, char* argv[])
{
    std::string servername = SERVERNAME;   
    unsigned short int port = PORT;
    
    std::string currentParam;
    
    if (argc > 1)
    {
        for (int i = 1; i < argc; ++i)
        {
            try
            {
                currentParam = argv[i];
            }
            
            catch (const std::exception& e)
            {
                continue;
            }
            
            if (currentParam == "-s")
            {
                try
                {
                    servername = std::string(argv[i + 1]);
                }
                
                catch (const std::exception& e)
                {
                    servername = SERVERNAME;
                }
            }
            
            else if (currentParam == "-p")
            {             
                try
                {
                    port = static_cast<unsigned short int>(std::stoul(argv[i + 1]));
                }
                
                catch (const std::exception& e)
                {
                    port = PORT;
                }
            }
        }
    }
    
    BaseController_RPi* baseControllerPtr = nullptr;
    
    try
    {
        baseControllerPtr = new BaseController_RPi(servername, port);
    }
    
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        
        delete baseControllerPtr;
        
        return 1;
    }
    
    std::thread listenerThread(&BaseController_RPi::listenforBLEDevices, baseControllerPtr);
    
    std::thread senderThread(&BaseController_RPi::sendDataPeriodically, baseControllerPtr);
    
    std::thread monitorThread(&BaseController_RPi::monitorSenderThread, baseControllerPtr);
    
    std::cout << "Enter 'q' (quit), 'e' (exit) or 'c' (close) to end the program..." << std::endl;
    
    std::string inputLine;
    
    std::getline(std::cin, inputLine);
    
    while ((inputLine != "q") && (inputLine != "e") && (inputLine != "c"))
        std::getline(std::cin, inputLine);
    
    baseControllerPtr->finalise();
    
    listenerThread.join();    
    monitorThread.join();
    senderThread.join();
    
    delete baseControllerPtr;

	return 0;
}