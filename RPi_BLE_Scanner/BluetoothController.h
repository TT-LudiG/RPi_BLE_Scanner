#ifndef BLUETOOTHCONTROLLER_H
#define BLUETOOTHCONTROLLER_H

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

class BluetoothController
{
private:    
    int _hciDeviceHandle, _hciStatus;
    
    struct hci_request getHCIRequest_BLE(uint16_t ocf, int clen, void* status, void* cparam);
	
public:   
    BluetoothController(void);
    ~BluetoothController(void);

    void openHCIDevice_Default(void);

    void startHCIScan_BLE(void);

    void stopHCIScan_BLE(void);

    void closeHCIDevice(void);
    
    int readDeviceInput(uint8_t* outputBuffer, uint16_t outputLength);
};

#endif