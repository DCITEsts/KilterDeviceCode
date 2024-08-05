#pragma once

#ifndef GLOBALPINS_H
#define GLOBALPINS_H

#include <string>

#pragma region DeviceStateGlobals

struct DeviceState
{
    // Active means limit switch is pressed
    bool bSeatLimitActive;
    bool bBackLimitActive;
    bool bSeatExtensionLimitActive;

    //Below is not used currently will be for when sensors are added

    int MainPivotAngle;
    int SeatExtension;
    int MidBackExtension;
};

extern struct DeviceState CurrentDeviceState;

struct ActuatorDefinition
{
    int ActuatorID;
    int PinA;
    int PinB;
    int ActuatorState;
};

#define StoppedState 000
#define ExtendingState 111
#define RetractingState 222

#define PivotActID 1
#define SeatActID 2
#define MidBackActID 3

extern struct ActuatorDefinition PivotActuator;
extern struct ActuatorDefinition SeatExtensionActuator;
extern struct ActuatorDefinition MidBackActuator;

#pragma endregion

#pragma region InputPinDefs

#define SeatLimitSwitch 25
#define SeatExtensionLimitSwitch 17
#define BackLimitSwitch 16

#pragma endregion

#pragma region ActuatorPinDefs

//Actuator Pin def's
#define SeatActuatorA 23
#define SeatActuatorB 22

#define PivotActuatorA 26
#define PivotActuatorB 27

#define MidBackActuatorA 19
#define MidBackActuatorB 18

#pragma endregion

#pragma region BLEGlobals

extern std::string LatestUUIDWritten;
extern int LatestReadValue;

//request for actuators below
#define PivotControlStopRequest 10
#define SeatControlStopRequest 20
#define MidBackControlStopRequest 30

#define PivotControlExtendRequest 11
#define SeatControlExtendRequest 21
#define MidBackControlExtendRequest 31

#define PivotControlRetractRequest 12
#define SeatControlRetractRequest 22
#define MidBackControlRetractRequest 32

#define StopAllMovementRequest 01
#define HomeRequest 44
#define StateRequest 66
#define InternalErrorRequest 34404
#define NoCommand 0

extern int CurrentRequest;

#pragma endregion

#endif