#include <cerrno>
#include <cstring>

#include <unistd.h>

#include "BluetoothController.h"
#include "BluetoothExceptions.h"

BluetoothController::BluetoothController(void)
{
    openHCIDevice_Default();
}

BluetoothController::~BluetoothController(void)
{ 
    closeHCIDevice();
}

void BluetoothController::openHCIDevice_Default(void)
{
    if ((_hciDeviceHandle = hci_open_dev(hci_get_route(NULL))) < 0)
	{
    	BluetoothExceptions::HCIDeviceDefaultOpenException e(std::strerror(errno));
	}
}

void BluetoothController::closeHCIDevice(void) const
{
    hci_close_dev(_hciDeviceHandle);
}

void BluetoothController::startHCIScan_BLE(void)
{
    // Set HCI BLE scan parameters.
	
    le_set_scan_parameters_cp scanParams;
    
    std::memset(static_cast<void*>(&scanParams), 0, sizeof(scanParams));
    
    scanParams.type = 0x00;
    scanParams.interval = htobs(0x0010);
    scanParams.window = htobs(0x0010);
    scanParams.own_bdaddr_type = 0x00; // Public Device Address (default).
    scanParams.filter = 0x00; // Accept all.

    struct hci_request setScanParams = getHCIRequest_BLE(OCF_LE_SET_SCAN_PARAMETERS, LE_SET_SCAN_PARAMETERS_CP_SIZE, static_cast<void*>(&_hciStatus), static_cast<void*>(&scanParams));
    
    if (hci_send_req(_hciDeviceHandle, &setScanParams, 1000) < 0)
    {
        hci_close_dev(_hciDeviceHandle);
        
        BluetoothExceptions::HCIBLEScanParamSetException e(std::strerror(errno));
    }

	// Set HCI BLE event mask.

    le_set_event_mask_cp eventMask;
    
    std::memset(static_cast<void*>(&eventMask), 0, sizeof(le_set_event_mask_cp));
    
    for (long int i = 0; i < 8; ++i)
        eventMask.mask[i] = 0xFF;

    struct hci_request setEventMask = getHCIRequest_BLE(OCF_LE_SET_EVENT_MASK, LE_SET_EVENT_MASK_CP_SIZE, static_cast<void*>(&_hciStatus), static_cast<void*>(&eventMask));
    
    if (hci_send_req(_hciDeviceHandle, &setEventMask, 1000) < 0)
    {
        hci_close_dev(_hciDeviceHandle);
        
        BluetoothExceptions::HCIBLEEventMaskSetException e(std::strerror(errno));
    }

	// Enable HCI BLE scan.

    le_set_scan_enable_cp scanEnable;
    
    std::memset(static_cast<void*>(&scanEnable), 0, sizeof(scanEnable));
    
    scanEnable.enable = 0x01; // Enable flag.
    scanEnable.filter_dup = 0x00; // Filtering disabled.

    struct hci_request setScanEnable = getHCIRequest_BLE(OCF_LE_SET_SCAN_ENABLE, LE_SET_SCAN_ENABLE_CP_SIZE, static_cast<void*>(&_hciStatus), static_cast<void*>(&scanEnable));
    
    if (hci_send_req(_hciDeviceHandle, &setScanEnable, 1000) < 0)
    {
        hci_close_dev(_hciDeviceHandle);
        
        BluetoothExceptions::HCIBLEScanEnableException e(std::strerror(errno));
    }

	// Set HCI BLE socket options.

    struct hci_filter newFilter;
    
    hci_filter_clear(&newFilter);
    hci_filter_set_ptype(HCI_EVENT_PKT, &newFilter);
    hci_filter_set_event(EVT_LE_META_EVENT, &newFilter);
    
    if (setsockopt(_hciDeviceHandle, SOL_HCI, HCI_FILTER, static_cast<void*>(&newFilter), sizeof(newFilter)) < 0)
    {
        hci_close_dev(_hciDeviceHandle);
        
        BluetoothExceptions::HCIBLESocketOptionsSetException e(std::strerror(errno));
    }
}

void BluetoothController::stopHCIScan_BLE(void)
{
    // Disable HCI BLE scan.
    
    le_set_scan_enable_cp scanEnable;

    std::memset(static_cast<void*>(&scanEnable), 0, sizeof(scanEnable));
    
    // Set the disable flag.
    
    scanEnable.enable = 0x00;

    struct hci_request setScanEnable = getHCIRequest_BLE(OCF_LE_SET_SCAN_ENABLE, LE_SET_SCAN_ENABLE_CP_SIZE, static_cast<void*>(&_hciStatus), static_cast<void*>(&scanEnable));
    
    if (hci_send_req(_hciDeviceHandle, &setScanEnable, 1000) < 0)
    {
        hci_close_dev(_hciDeviceHandle);
        
        BluetoothExceptions::HCIBLEScanDisableException e(std::strerror(errno));
    }
}

struct hci_request BluetoothController::getHCIRequest_BLE(const unsigned short int ocf, const long int clen, void* status, void* cparam)
{
    struct hci_request hciRequest;
    
    std::memset(static_cast<void*>(&hciRequest), 0, sizeof(hciRequest)) ;
    
    hciRequest.ogf = OGF_LE_CTL;
    hciRequest.ocf = ocf;
    
    hciRequest.cparam = cparam;
    hciRequest.clen = clen;
    
    hciRequest.rparam = status;
    hciRequest.rlen = 1;
    
    return hciRequest;
}

long int BluetoothController::readDeviceInput(unsigned char* outputBuffer, const unsigned short int bufferLength) const
{  
    return read(_hciDeviceHandle, static_cast<void*>(outputBuffer), bufferLength);
}