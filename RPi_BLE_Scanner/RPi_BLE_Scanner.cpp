#include <exception>
#include <iostream>
#include <thread>

#include "BaseController_RPi.h"

#define SERVERNAME_GENERAL "vpn-wa-a.thermotrack.co.za"
#define SERVERNAME_DATA "whizzdev.dyndns.org"
#define PORT_GENERAL "49996"
#define PORT_DATA "9062"
#define DELAY_SENDER_LOOP_IN_SEC 300;

int main(int argc, char* argv[])
{
    // Read run-time arguments.
    
    std::string servername_general = SERVERNAME_GENERAL;
    std::string servername_data = SERVERNAME_DATA;
    std::string port_general = PORT_GENERAL;
    std::string port_data = PORT_DATA;
    unsigned long int delay_sender_loop_in_sec = DELAY_SENDER_LOOP_IN_SEC;
    
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
            
            std::size_t indexEqualsChar = currentParam.find('=');
            
            if (indexEqualsChar != std::string::npos)
            {
                std::string token = currentParam.substr(0, indexEqualsChar);
                std::string value = currentParam.substr(indexEqualsChar + 1);
                
                if (token == "-servername_general")
                    servername_general = value;
                
                else if (token == "-servername_data")
                    servername_data = value;
                
                else if (token == "-port_general")
                    port_general = value;
                
                else if (token == "-port_data")
                    port_data = value;
                
                else if (token == "-delay_sender_loop_in_sec")
                    delay_sender_loop_in_sec = std::stoul(value);
            }
        }
    }
    
    // Initialise the BaseController.
    
    BaseController_RPi* baseControllerPtr = nullptr;
    
    try
    {
        baseControllerPtr = new BaseController_RPi(servername_general, servername_data, port_general, port_data, delay_sender_loop_in_sec);
    }
    
    catch (const std::exception& e)
    {        
        if (baseControllerPtr != nullptr)
            delete baseControllerPtr;
        
        return 1;
    }
    
    // Create the listener, sender and monitor threads.

    std::thread listenerThread(&BaseController_RPi::listenForBLEDevices, baseControllerPtr);
    
    std::thread senderThread(&BaseController_RPi::sendDataPeriodically, baseControllerPtr);
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    std::thread monitorThread(&BaseController_RPi::monitorSenderThread, baseControllerPtr);
    
    // Run the UI loop.
    
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