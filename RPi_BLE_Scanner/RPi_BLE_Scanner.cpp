#include <exception>
#include <iostream>
#include <thread>

#include "BaseController.h"

int main(void)
{
    BaseController* baseControllerPtr;
    
    std::string inputLine;
    
    try
    {
        baseControllerPtr = new BaseController();
    }
    
    catch (const std::exception& e)
    {
    }
    
    std::thread listenerThread(&BaseController::listenforBLEDevices, baseControllerPtr);
    
    std::getline(std::cin, inputLine);
    
    while (inputLine != "q")
    {
        std::getline(std::cin, inputLine);
    }
    
    baseControllerPtr->finalise();
    
    listenerThread.join();
    
    delete baseControllerPtr;

	return 0;
}