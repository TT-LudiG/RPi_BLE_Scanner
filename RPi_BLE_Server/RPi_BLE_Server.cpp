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

    catch (std::exception e)
    {
        std::cerr << e.what() << std::endl;

        delete baseControllerPtr;

        return 1;
    }

    while (true)
    {
        baseControllerPtr->update();

        // Sleep for one 1/10th of a second.

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    delete baseControllerPtr;

    return 0;
}