#include "stdafx.h"

#include <exception>
#include <iostream>

#include "BaseController.h"

#pragma comment (lib, "Ws2_32.lib")

int main()
{
    BaseController* baseControllerPtr = nullptr;

    try
    {
        baseControllerPtr = new BaseController();
    }

    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;

        delete baseControllerPtr;

        return 1;
    }

    std::thread monitorThread = std::thread(&BaseController::monitorClients, baseControllerPtr);

    std::cout << "Enter 'q' (quit), 'e' (exit) or 'c' (close) to end the program..." << std::endl;
    
    std::string inputLine;
    
    std::getline(std::cin, inputLine);

    while ((inputLine != "q") && (inputLine != "e") && (inputLine != "c"))
        std::getline(std::cin, inputLine);

    baseControllerPtr->finalise();

    monitorThread.join();

    delete baseControllerPtr;

    return 0;
}