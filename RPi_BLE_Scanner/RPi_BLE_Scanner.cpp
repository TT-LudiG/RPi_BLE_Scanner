#include <exception>
#include <iostream>
#include <thread>

#include "BaseController_RPi.h"

int main(void)
{
    BaseController_RPi* baseControllerPtr = nullptr;
    
    try
    {
        baseControllerPtr = new BaseController_RPi();
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