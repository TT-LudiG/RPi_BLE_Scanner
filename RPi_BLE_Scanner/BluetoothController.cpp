#include <unistd.h>

#include "BluetoothController.h"

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
//    	snprintf(hciState.ErrorMessage, sizeof(hciState.ErrorMessage), "Could not open device: %s", strerror(errno));
	}
}

void BluetoothController::startHCIScan_BLE(void)
{
    // Set BLE scan parameters.
	
    le_set_scan_parameters_cp scanParams;
    
    memset(&scanParams, 0, sizeof(scanParams));
    
    scanParams.type 			= 0x00; 
    scanParams.interval 		= htobs(0x0010);
    scanParams.window 			= htobs(0x0010);
    scanParams.own_bdaddr_type 	= 0x00; // Public Device Address (default).
    scanParams.filter 			= 0x00; // Accept all.

    struct hci_request setScanParams = getHCIRequest_BLE(OCF_LE_SET_SCAN_PARAMETERS, LE_SET_SCAN_PARAMETERS_CP_SIZE, &_hciStatus, &scanParams);
    
    if (hci_send_req(_hciDeviceHandle, &setScanParams, 1000) < 0)
    {
        hci_close_dev(_hciDeviceHandle);
    	
//        perror("Failed to set scan parameters data.");
    }

	// Set BLE events report mask.

    le_set_event_mask_cp eventMask;
    
    memset(&eventMask, 0, sizeof(le_set_event_mask_cp));
    
    int i = 0;
    
    for (i = 0; i < 8; i++)
        eventMask.mask[i] = 0xFF;

    struct hci_request setEventMask = getHCIRequest_BLE(OCF_LE_SET_EVENT_MASK, LE_SET_EVENT_MASK_CP_SIZE, &_hciStatus, &eventMask);
    
    if (hci_send_req(_hciDeviceHandle, &setEventMask, 1000) < 0)
    {
        hci_close_dev(_hciDeviceHandle);
    	
//        perror("Failed to set event mask.");
    }

	// Enable scanning.

    le_set_scan_enable_cp scanEnable;
    
    memset(&scanEnable, 0, sizeof(scanEnable));
    
    scanEnable.enable = 0x01;	// Enable flag.
    scanEnable.filter_dup = 0x00; // Filtering disabled.

    struct hci_request setScanEnable = getHCIRequest_BLE(OCF_LE_SET_SCAN_ENABLE, LE_SET_SCAN_ENABLE_CP_SIZE, &_hciStatus, &scanEnable);
    
    if (hci_send_req(_hciDeviceHandle, &setScanEnable, 1000) < 0)
    {
        hci_close_dev(_hciDeviceHandle);
    	
//        perror("Failed to enable scan.");
    }

	// Get Results.

    struct hci_filter newFilter;
    
    hci_filter_clear(&newFilter);
    hci_filter_set_ptype(HCI_EVENT_PKT, &newFilter);
    hci_filter_set_event(EVT_LE_META_EVENT, &newFilter);
    
    if (setsockopt(_hciDeviceHandle, SOL_HCI, HCI_FILTER, &newFilter, sizeof(newFilter)) < 0)
    {
        hci_close_dev(_hciDeviceHandle);
    	
//        perror("Could not set socket options\n");
    }
}

void BluetoothController::stopHCIScan_BLE(void)
{
    // Disable scanning.
    
    le_set_scan_enable_cp scanEnable;

    memset(&scanEnable, 0, sizeof(scanEnable));
    
    scanEnable.enable = 0x00;	// Disable flag.

    struct hci_request setScanEnable = getHCIRequest_BLE(OCF_LE_SET_SCAN_ENABLE, LE_SET_SCAN_ENABLE_CP_SIZE, &_hciStatus, &scanEnable);
    
    if (hci_send_req(_hciDeviceHandle, &setScanEnable, 1000) < 0)
    {
        hci_close_dev(_hciDeviceHandle);
    	
//        perror("Failed to disable scan.");
    }
}

void BluetoothController::closeHCIDevice(void)
{
    hci_close_dev(_hciDeviceHandle);
}

struct hci_request BluetoothController::getHCIRequest_BLE(uint16_t ocf, int clen, void* status, void* cparam)
{
    struct hci_request hciRequest;
    
    memset(&hciRequest, 0, sizeof(hciRequest));
    
    hciRequest.ogf = OGF_LE_CTL;
    hciRequest.ocf = ocf;
    
    hciRequest.cparam = cparam;
    hciRequest.clen = clen;
    
    hciRequest.rparam = status;
    hciRequest.rlen = 1;
    
    return hciRequest;
}

int BluetoothController::readDeviceInput(uint8_t* outputBuffer, uint16_t outputLength)
{  
    return read(_hciDeviceHandle, outputBuffer, outputLength);
}