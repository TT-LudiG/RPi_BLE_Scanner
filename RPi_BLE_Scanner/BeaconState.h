#ifndef BEACONSTATE_H
#define BEACONSTATE_H

struct BeaconState
{
//    float Value;    
//    float Battery;
    
    unsigned short int Temperature;
    unsigned short int Humidity;
    unsigned char Battery;
    
//    BeaconState(float value, float battery): Value(value), Battery(battery) {}
    
    BeaconState(unsigned short int temperature, unsigned short int humidity, unsigned char battery): Temperature(temperature), Humidity(humidity), Battery(battery) {}
};

#endif