#include "stdafx.h"

#include <exception>
#include <iostream>

#include "BaseController.h"

#pragma comment (lib, "Ws2_32.lib")

#define PORT 2226

int main(int argc, char* argv[])
{
    unsigned short int port = PORT;

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
            
            if (currentParam == "-p")
            {
                try
                {
                    port = static_cast<unsigned short int>(std::stoul(argv[i + 1]));
                }
                
                catch (...)
                {
                    port = PORT;
                }
            }
        }
    }

    BaseController* baseControllerPtr = nullptr;

    try
    {
        baseControllerPtr = new BaseController(port);
    }

    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;

        delete baseControllerPtr;

        return 1;
    }

    std::thread monitorThread = std::thread(&BaseController::monitorThreads, baseControllerPtr);

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