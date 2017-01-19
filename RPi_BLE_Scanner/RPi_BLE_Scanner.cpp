#include <exception>
#include <iostream>
#include <thread>

#include "BaseController_RPi.h"

int main(void)
{
    BaseController* baseControllerPtr = nullptr;
    
    try
    {
        baseControllerPtr = new BaseController();
    }
    
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        
        return 1;
    }
    
    std::thread listenerThread(&BaseController::listenforBLEDevices, baseControllerPtr);
    
    std::thread senderThread(&BaseController::sendDataPeriodically, baseControllerPtr);
    
    std::string inputLine;
    
    std::getline(std::cin, inputLine);
    
    while (inputLine != "q")
    {
        std::getline(std::cin, inputLine);
    }
    
    baseControllerPtr->finalise();
    
    listenerThread.join();
    senderThread.join();
    
    delete baseControllerPtr;

	return 0;
}