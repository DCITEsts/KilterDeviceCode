
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

void StopActuator(ActuatorDefinition *Actuator);//stops the specified actuator

//returns true if the actuator can move in the direction of the position or is at position and sets the actuator to move in that direction. 
//Returns false if the actuator cannot move in the required direction to go towards the goal
//Needs to be called in loop to actually complete the move will not complete the move in one call only goes in the direction of the the currently set goal
bool ActuatorTryGoToPosition(ActuatorDefinition *Actuator, DeviceState *State); 

void StopAllMotion();// stops all of the actuators from moving

private:

// below are internal functions to determine if actuators can move
// true means that actuator can move in the direction, false it can't

bool CanExtendPivot(DeviceState *State);
bool CanRetractPivot(DeviceState *State);

bool CanExtendSeat(DeviceState *State);
bool CanRetractSeat(DeviceState *State);

bool CanExtendMidback(DeviceState *State);
bool CanRetractMidBack(DeviceState *State);

};
#endif