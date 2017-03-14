#ifndef BEACONSTATE_H
#define BEACONSTATE_H

struct BeaconState
{
    float Value;    
    float Battery;
    
    BeaconState(float value, float battery): Value(value), Battery(battery) {}
};

#endif