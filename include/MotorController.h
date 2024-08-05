
#pragma once

#ifndef MOTORCONTROLLER_H
#define MOTORCONTROLLER_H

#include "GlobalPins.h"

struct MotorDirection
{
    int InputPin1Value;
    int InputPin2Value;
};

class MotorController
{

private:

const struct MotorDirection ExtendAct = {1,0}; // Will check wiring to insure this works correctly actuators must match
const struct MotorDirection RetractAct = {0,1}; // Will check wiring to insure this works correctly actuators must match
const struct MotorDirection StopAct = {0,0};

public:

bool ExtendActuator(ActuatorDefinition *Actuator, DeviceState *State);
bool RetractActuator(ActuatorDefinition *Actuator, DeviceState *State);
void StopActuator(ActuatorDefinition *Actuator);
void StopAllMotion();

private:

bool CanExtendPivot(DeviceState *State);
bool CanRetractPivot(DeviceState *State);

bool CanExtendSeat(DeviceState *State);
bool CanRetractSeat(DeviceState *State);

bool CanExtendMidback(DeviceState *State);
bool CanRetractMidBack(DeviceState *State);

};
#endif