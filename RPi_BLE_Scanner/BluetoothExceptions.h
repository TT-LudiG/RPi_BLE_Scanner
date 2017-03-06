#ifndef BLUETOOTH_EXCEPTIONS_H
#define BLUETOOTH_EXCEPTIONS_H

#include <exception>
#include <string>

namespace BluetoothExceptions
{
    // EXCEPTION_BLUE_0

    class HCIDeviceDefaultOpenException: public std::exception
    {
    private:
        const std::string _error;
	
    public:
        HCIDeviceDefaultOpenException(const std::string error): _error(error) {}
	
        virtual const char* what() const throw()
        {
            std::string message = "EXCEPTION_BLUE_0: Failed to open the default HCI device, with error: " + _error;
		
            return message.c_str();
        }
    };

    // EXCEPTION_BLUE_1

    class HCIBLEScanParamSetException: public std::exception
    {
    private:
        const std::string _error;
	
    public:
        HCIBLEScanParamSetException(const std::string error): _error(error) {}
	
        virtual const char* what() const throw()
        {
            std::string message = "EXCEPTION_BLUE_1: Failed to set the HCI BLE scan parameters, with error: " + _error;
		
            return message.c_str();
        }
    };

    // EXCEPTION_BLUE_2

    class HCIBLEEventMaskSetException: public std::exception
    {
    private:
        const std::string _error;
	
    public:
        HCIBLEEventMaskSetException(const std::string error): _error(error) {}
	
        virtual const char* what() const throw()
        {
            std::string message = "EXCEPTION_BLUE_2: Failed to set the HCI BLE event mask, with error: " + _error;
		
            return message.c_str();
        }
    };

    // EXCEPTION_BLUE_3

    class HCIBLEScanEnableException: public std::exception
    {
    private:
        const std::string _error;
	
    public:
        HCIBLEScanEnableException(const std::string error): _error(error) {}
	
        virtual const char* what() const throw()
        {
            std::string message = "EXCEPTION_BLUE_3: Failed to enable the HCI BLE scan, with error: " + _error;
		
            return message.c_str();
        }
    };

    // EXCEPTION_BLUE_4

    class HCIBLESocketOptionsSetException: public std::exception
    {
    private:
        const std::string _error;
	
    public:
        HCIBLESocketOptionsSetException(const std::string error): _error(error) {}
	
        virtual const char* what() const throw()
        {
            std::string message = "EXCEPTION_BLUE_4: Failed to set the HCI BLE socket options, with error: " + _error;
		
            return message.c_str();
        }
    };

    // EXCEPTION_BLUE_5

    class HCIBLEScanDisableException: public std::exception
    {
    private:
        const std::string _error;
	
    public:
        HCIBLEScanDisableException(const std::string error): _error(error) {}
	
        virtual const char* what() const throw()
        {
            std::string message = "EXCEPTION_BLUE_5: Failed to disable the HCI BLE scan, with error: " + _error;
		
            return message.c_str();
        }
    };
}

#endif