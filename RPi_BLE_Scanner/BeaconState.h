#ifndef BEACONSTATE_H
#define BEACONSTATE_H

struct BeaconState
{
    short int Temperature;    
    unsigned short int Humidity;
    unsigned char Battery;
    
    BeaconState(short int temperature, unsigned short int humidity, unsigned char battery): Temperature(temperature), Humidity(humidity), Battery(battery) {}
};

#endif