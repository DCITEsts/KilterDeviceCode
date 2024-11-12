
#pragma once

#ifndef MOTORCONTROLLER_H
#define MOTORCONTROLLER_H

#include "GlobalPins.h"
#include "ESP32Encoder.h"

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

bool ExtendActuator(ActuatorDefinition *Actuator, DeviceState *State);// checks if actuator can be extended and then extends if it can
bool RetractActuator(ActuatorDefinition *Actuator, DeviceState *State);//checks if actuator can be retracted and then retracts if it can
void StopActuator(ActuatorDefinition *Actuator);
bool ActuatorTryGoToPosition(ActuatorDefinition *Actuator, DeviceState *State); //returns true if the actuator can move in the direction of the position or is at position and sets the actuator to move in that direction.
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