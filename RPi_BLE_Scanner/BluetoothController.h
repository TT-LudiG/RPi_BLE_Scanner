#ifndef BLUETOOTHCONTROLLER_H
#define BLUETOOTHCONTROLLER_H

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

class BluetoothController
{
private:    
    long int _hciDeviceHandle, _hciStatus;
    
    void openHCIDevice_Default(void);
    void closeHCIDevice(void) const;
    
    static struct hci_request getHCIRequest_BLE(const unsigned short int ocf, const long int clen, void* status, void* cparam);
	
public:   
    BluetoothController(void);    
    ~BluetoothController(void);

    void startHCIScan_BLE(void);
    void stopHCIScan_BLE(void);
    
    long int readDeviceInput(unsigned char* outputBuffer, const unsigned short int bufferLength) const;
};

#endif