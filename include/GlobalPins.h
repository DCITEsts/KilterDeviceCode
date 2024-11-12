#pragma once

#ifndef GLOBALPINS_H
#define GLOBALPINS_H

#include <string>
#include <Arduino.h>

#pragma region DeviceStateGlobals

struct DeviceState
{
    // Active means limit switch is pressed
    bool bSeatLimitActive;
    bool bBackLimitActive;
    bool bSeatExtensionLimitActive;
    bool bMidBackLimitActive;

    int MainPivotAngle; //Up to date Reading of encoder
    bool bMainPivotIsGoingToLocation; //if true the encoder is trying to reach a location
    int CurrentMainPivotGoal;   //the goal set for this encoder to go to if it is trying to reach location

    int SeatExtension;
    bool bSeatExtensionIsGoingToLocation;//if true the encoder is trying to reach a location
    int CurrentSeatExtensionGoal;//the goal set for this encoder to go to if it is trying to reach location

    int MidBackExtension;//Up to date Reading of encoder
    bool bMidBackIsgoingToLocation;//the goal set for this encoder to go to if it is trying to reach location
    int CurrentMidBackGoal;//if true the encoder is trying to reach a location
    
};

extern struct DeviceState CurrentDeviceState;//update from anywhere if you want but this should be the primary device state

struct ActuatorDefinition
{
    int ActuatorID;
    int PinA;
    int PinB;
    int ActuatorState;
};

//states for individual actuaotors
#define StoppedState 000
#define ExtendingState 111
#define RetractingState 222

//Id Codes for individual actuators
#define PivotActID 1
#define SeatActID 2
#define MidBackActID 3

//Globals to represent the individual actuators
extern struct ActuatorDefinition PivotActuator;
extern struct ActuatorDefinition SeatExtensionActuator;
extern struct ActuatorDefinition MidBackActuator;

#pragma endregion

#pragma region InputPinDefs

//Pins will need pulldown resistor on ESP32 since there is no internal pulldown on these pins
#define SeatLimitSwitch 34 
#define SeatExtensionLimitSwitch 35 
#define BackLimitSwitch 36 
#define MidBackLimitSwitch 39 

#pragma endregion

#pragma region ActuatorPinDefs

//Actuator Pin def's
#define SeatActuatorA 23
#define SeatActuatorB 22

#define PivotActuatorA 26
#define PivotActuatorB 27

#define MidBackActuatorA 19
#define MidBackActuatorB 18

//Encoder Pin defs
#define MidbackEncoderPinA 32
#define SeatAngleEncoderPinA 21
#define SeatExtensionEncoderPinA 16

#define MidbackEncoderPinB 33
#define SeatAngleEncoderPinB 25
#define SeatExtensionEncoderPinB 17


extern volatile DRAM_ATTR int MidBackEncoderVal;
extern volatile DRAM_ATTR int SeatAngleEncoderVal;
extern volatile DRAM_ATTR int SeatExtensionEncoderVal;

#pragma endregion

#pragma region BLEGlobals

extern std::string LatestUUIDWritten;
extern int LatestReadValue;// latest value of the request characteristic

//incoming device requests below
//These are the commands that you can send the device to have it do stuff
//these Requests are read from the incoming requests characterstic and used to change sate
#define PivotControlStopRequest 10
#define SeatControlStopRequest 20
#define MidBackControlStopRequest 30

#define PivotControlExtendRequest 11
#define SeatControlExtendRequest 21
#define MidBackControlExtendRequest 31

#define PivotControlRetractRequest 12
#define SeatControlRetractRequest 22
#define MidBackControlRetractRequest 32

#define PivotControlGoToPositionRequest 13 
#define SeatExtensionGoToPositionRequest 23
#define MidBackGoToPositionRequest 33

#define StopAllMovementRequest 01
#define HomeRequest 44
#define StateRequest 66
#define UpdatePositions 77
#define NoCommand 0

//Outgoing device state Requests Below
//These are commands you can read from the device that determine state
//These will be written to the outgoing request characteristic and used to communicate state to the BLE
#define InternalErrorRequest 34404
#define IdleRequest 11
#define ReceivedCommandRequest 12

#define FailedToReachSeatAngleGoalLocationRequest 21
#define SucceededToReachSeatAngleGoalLocationRequest 31
#define FailedToReachSeatExtensionGoalLocationRequest 22
#define SucceededToReachSeatExtensionGoalLocationRequest 32
#define FailedToReachMidBackGoalLocationRequest 23
#define SucceededToReachMidBackGoalLocationRequest 33

extern int CurrentRequest;//the current incoming request read from the incoming request characteristic
extern int CurrentState;// the current outgoing state written to the outgoing request characteristic

#pragma endregion

#endif