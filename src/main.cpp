#include <Arduino.h>
#include "GlobalPins.h"
#include "MotorController.h"
#include "InterruptHandling.h"
#include "BLEController.h"
#include "strings.h"
#include "ESP32Encoder.h"
#include "string.h"

//Encoder Vars
ESP32Encoder MidBackEncoder;
ESP32Encoder SeatAngleEncoder;
ESP32Encoder SeatExtensionEncoder;

void UpdateDevicestate(); // helper function to update all switch locations and positions

void ThrowError(String ErrorString); // prints an error string to the serial and lets BLE know about internal error but no details over ble

void StopAllActuatorsMovingToSpecificLocation(); // helper function to cancel any actuators currently moving to specific locations.  Actuators that haven't completed their move will fail.

//Motor Controller and BLE Controller vars for main
static MotorController CurrentMotorController; //one controller handles all 3 motors
static BLEController CurrentBLEController;

// notifies the ble device of new outgoing request state will only send new states if they don't match current state.  
//All state changes should ideally be handled by this function so that state gets written write
void SetCurrentOutgoingRequest(int NewRequest); 

bool bWantsHome = false; // set to start device going home.  Homing will be handled in the main loop

void setup()
{

  Serial.begin(115200);

  Serial.println("Setting up");

#pragma region Pinsetup

  // setup the limit switches
  pinMode(SeatLimitSwitch, INPUT);
  pinMode(SeatExtensionLimitSwitch, INPUT);
  pinMode(BackLimitSwitch, INPUT);
  pinMode(MidBackLimitSwitch, INPUT);

  // setup the actuator pins
  pinMode(PivotActuatorA, OUTPUT);
  pinMode(PivotActuatorB, OUTPUT);
  pinMode(SeatActuatorA, OUTPUT);
  pinMode(SeatActuatorB, OUTPUT);
  pinMode(MidBackActuatorA, OUTPUT);
  pinMode(MidBackActuatorB, OUTPUT);

  // Initialize the actuators to the powered off state
  digitalWrite(PivotActuatorA, LOW);
  digitalWrite(PivotActuatorB, LOW);
  digitalWrite(SeatActuatorA, LOW);
  digitalWrite(SeatActuatorB, LOW);
  digitalWrite(MidBackActuatorA, LOW);
  digitalWrite(MidBackActuatorB, LOW);

#pragma endregion

  CurrentMotorController.StopAllMotion(); // make sure we have everything in sync with the motorcontroller and actual actuator pins

#pragma region EncoderSetup
  ESP32Encoder::useInternalWeakPullResistors = puType::up;

  MidBackEncoder.attachHalfQuad(MidbackEncoderPinA, MidbackEncoderPinB);
  SeatAngleEncoder.attachHalfQuad(SeatAngleEncoderPinA, SeatAngleEncoderPinB);
  SeatExtensionEncoder.attachHalfQuad(SeatExtensionEncoderPinA, SeatExtensionEncoderPinB);

  //Begin encoderscounting from 0
  MidBackEncoder.resumeCount();
  SeatAngleEncoder.resumeCount();
  SeatExtensionEncoder.resumeCount();

  MidBackEncoder.clearCount();
  SeatAngleEncoder.clearCount();
  SeatExtensionEncoder.clearCount();

  CurrentDeviceState.MidBackExtension = MidBackEncoder.getCount();
  CurrentDeviceState.MainPivotAngle = SeatAngleEncoder.getCount();
  CurrentDeviceState.SeatExtension = SeatExtensionEncoder.getCount();

  //set the initial count on BLE connection.  Just a sanity check since they should all be 0 as long as they are all connected.
  CurrentBLEController.WriteMidbackPositionCharacteristic(std::to_string(MidBackEncoder.getCount()));
  CurrentBLEController.WriteSeatAngleCharacteristic(std::to_string(SeatAngleEncoder.getCount()));
  CurrentBLEController.WriteSeatExtensionPositionCharacteristic(std::to_string(MidBackEncoder.getCount()));

  #pragma endregion


  // read the initial states of all the limit switches and sync them up with the flags
  isSeatLimitActive = (digitalRead(SeatLimitSwitch) == HIGH);
  isSeatExtLimitActive = (digitalRead(SeatExtensionLimitSwitch) == HIGH);
  isBackLimitActive = (digitalRead(BackLimitSwitch) == HIGH);
  isMidBackLimitActive = (digitalRead(MidBackLimitSwitch) == HIGH);

  CurrentBLEController.BLEsetup(); // run the BLE Setup for our primary controller

  UpdateDevicestate();//initial state update
  SetCurrentOutgoingRequest(IdleRequest);

  // attach all the interrups for the limit switches
  attachInterrupt(digitalPinToInterrupt(SeatExtensionLimitSwitch), onSeatExtLimitActive, CHANGE);
  attachInterrupt(digitalPinToInterrupt(SeatLimitSwitch), onSeatLimitActive, CHANGE);
  attachInterrupt(digitalPinToInterrupt(BackLimitSwitch), onBackLimitActive, CHANGE);
  attachInterrupt(digitalPinToInterrupt(MidBackLimitSwitch), onMidBackLimitActive, CHANGE);

  Serial.println("Setup Complete");
}

void loop()
{

#pragma region UpdateDeviceStatusSetup
  CurrentRequest = LatestReadValue; // consume buffered command from ble
  LatestReadValue = NoCommand;      // reset command buffer

  if (Serial.available() > 0) // if the serial has commands overwrite the BLE Command with the serial.  Priority given to serial commands if available
  {
    CurrentRequest = Serial.parseInt();
    Serial.println("Serial Commmand Received:");
    Serial.print(CurrentRequest);
    Serial.println("");
    Serial.flush();
    while (Serial.available() > 0)
    {
      Serial.read();
    }
  }

  UpdateDevicestate();

#pragma endregion

#pragma region HandleActuatorsGoingToLocations

  //only enter if some actuator wants to go to position
  if (CurrentDeviceState.bMainPivotIsGoingToLocation || CurrentDeviceState.bSeatExtensionIsGoingToLocation || CurrentDeviceState.bMidBackIsgoingToLocation)
  {
    if (CurrentDeviceState.bMainPivotIsGoingToLocation) // handle main pivot actuator going to location
    {
      CurrentBLEController.WriteSeatAngleCharacteristic(std::to_string(CurrentDeviceState.MainPivotAngle));
      bool bGoingToLocation = CurrentMotorController.ActuatorTryGoToPosition(&PivotActuator, &CurrentDeviceState); //check if we can actually go in the direction of the location
      if (bGoingToLocation)
      {
        if (CurrentDeviceState.MainPivotAngle == CurrentDeviceState.CurrentMainPivotGoal)// Check if we're at the location already
        {
          CurrentBLEController.WriteOutgoingCommsCharacteristic(std::to_string(SucceededToReachSeatAngleGoalLocationRequest));
          Serial.println("got to location Main Pivot");
          CurrentDeviceState.bMainPivotIsGoingToLocation = false;
        }
      }
      else
      {
        CurrentBLEController.WriteOutgoingCommsCharacteristic(std::to_string(FailedToReachSeatAngleGoalLocationRequest));
        CurrentDeviceState.bMainPivotIsGoingToLocation = false;
        Serial.println("failed go to location Main Pivot");
      }
    }
    if (CurrentDeviceState.bSeatExtensionIsGoingToLocation)// handle Seat extension actuator going to location
    {
      CurrentBLEController.WriteSeatExtensionPositionCharacteristic(std::to_string(CurrentDeviceState.MainPivotAngle));
      bool bGoingToLocation = CurrentMotorController.ActuatorTryGoToPosition(&SeatExtensionActuator, &CurrentDeviceState);//check if we can actually go in the direction of the location
      if (bGoingToLocation)
      {
        if (CurrentDeviceState.SeatExtension == CurrentDeviceState.CurrentSeatExtensionGoal)// Check if we're at the location already
        {
          CurrentBLEController.WriteOutgoingCommsCharacteristic(std::to_string(SucceededToReachSeatExtensionGoalLocationRequest));
          CurrentDeviceState.bSeatExtensionIsGoingToLocation = false;
          Serial.println("got to location Seat Extension");
        }
      }
      else
      {
        CurrentBLEController.WriteOutgoingCommsCharacteristic(std::to_string(FailedToReachSeatExtensionGoalLocationRequest));
        CurrentDeviceState.bSeatExtensionIsGoingToLocation = false;
        Serial.println("Failed go to location Seat Extension");
      }
    }
    if (CurrentDeviceState.bMidBackIsgoingToLocation)// handle MidBack actuator going to location
    {
      CurrentBLEController.WriteMidbackPositionCharacteristic(std::to_string(CurrentDeviceState.MidBackExtension));
      bool bGoingToLocation = CurrentMotorController.ActuatorTryGoToPosition(&MidBackActuator, &CurrentDeviceState);//check if we can actually go in the direction of the location
      if (bGoingToLocation)
      {
        if (CurrentDeviceState.MidBackExtension == CurrentDeviceState.CurrentMidBackGoal)// Check if we're at the location already
        {
          CurrentBLEController.WriteOutgoingCommsCharacteristic(std::to_string(SucceededToReachMidBackGoalLocationRequest));
          CurrentDeviceState.bMidBackIsgoingToLocation = false;
          Serial.println("got to location MidBack");
        }
      }
    }
    else
    {
      CurrentBLEController.WriteOutgoingCommsCharacteristic(std::to_string(FailedToReachMidBackGoalLocationRequest));
      CurrentDeviceState.bMidBackIsgoingToLocation = false;
      Serial.println("Failed go to location MidBack");
    }
  }

#pragma endregion

#pragma region HandleHoming

  if (bWantsHome && !CurrentDeviceState.bSeatExtensionLimitActive)
  {
    Serial.println("Homing Seat Extension");
    CurrentMotorController.RetractActuator(&SeatExtensionActuator, &CurrentDeviceState); // if the seat is still extended and we're trying to home retract the seat first
  }
  if (bWantsHome && CurrentDeviceState.bSeatExtensionLimitActive && !CurrentDeviceState.bSeatLimitActive)
  {
    Serial.println("Homing Pivot");
    CurrentMotorController.RetractActuator(&PivotActuator, &CurrentDeviceState); // if the seat is not extended and we want home then retract the pivot to get to a seated position
  }
  if (CurrentDeviceState.bSeatExtensionLimitActive && CurrentDeviceState.bSeatLimitActive && !CurrentDeviceState.bBackLimitActive)
  {
    bWantsHome = false; // we're home! give up on homing if we're trying to do it.
  }

#pragma endregion

#pragma region HandleRequestsSwitch

  //Giant switch statement handles state transitions based on request written to currentrequest
  switch (CurrentRequest)
  {
  case NoCommand:
  {
    // no commands received just don't do anything
    SetCurrentOutgoingRequest(IdleRequest);
    break;
  }
  case StopAllMovementRequest:
  {
    SetCurrentOutgoingRequest(ReceivedCommandRequest);
    Serial.println("Stopping all movement Command");
    bWantsHome = false;
    StopAllActuatorsMovingToSpecificLocation(); // Cancel any calls to Move actuators to specific locations
    CurrentMotorController.StopAllMotion();
    CurrentBLEController.WriteMidbackPositionCharacteristic(std::to_string(MidBackEncoder.getCount()));
    CurrentBLEController.WriteSeatAngleCharacteristic(std::to_string(SeatAngleEncoder.getCount()));
    CurrentBLEController.WriteSeatExtensionPositionCharacteristic(std::to_string(MidBackEncoder.getCount()));
    break;
  }
  case PivotControlStopRequest:
  {
    SetCurrentOutgoingRequest(ReceivedCommandRequest);
    bWantsHome = false;
    StopAllActuatorsMovingToSpecificLocation(); // Cancel any calls to Move actuators to specific locations
    Serial.println("Stop Pivot Command");
    CurrentMotorController.StopActuator(&PivotActuator);
    CurrentBLEController.WriteSeatAngleCharacteristic(std::to_string(SeatAngleEncoder.getCount()));
    break;
  }

  case SeatControlStopRequest:
  {
    SetCurrentOutgoingRequest(ReceivedCommandRequest);
    bWantsHome = false;
    StopAllActuatorsMovingToSpecificLocation(); // Cancel any calls to Move actuators to specific locations
    Serial.println("Stop Seat Command");
    CurrentMotorController.StopActuator(&SeatExtensionActuator);
    CurrentBLEController.WriteSeatExtensionPositionCharacteristic(std::to_string(MidBackEncoder.getCount()));
    break;
  }

  case MidBackControlStopRequest:
  {
    SetCurrentOutgoingRequest(ReceivedCommandRequest);
    bWantsHome = false;
    StopAllActuatorsMovingToSpecificLocation(); // Cancel any calls to Move actuators to specific locations
    Serial.println("Stop MB Command");
    CurrentMotorController.StopActuator(&MidBackActuator);
    CurrentBLEController.WriteMidbackPositionCharacteristic(std::to_string(MidBackEncoder.getCount()));
    break;
  }

  case PivotControlExtendRequest:
  {
    SetCurrentOutgoingRequest(ReceivedCommandRequest);
    StopAllActuatorsMovingToSpecificLocation(); // Cancel any calls to Move actuators to specific locations
    Serial.println("Extend Pivot Command");
    bWantsHome = false;
    bool PivotCanExtend = CurrentMotorController.ExtendActuator(&PivotActuator, &CurrentDeviceState);
    if (PivotCanExtend)
    {
      Serial.println("Extending Pivot from serial");
    }
    else
    {
      ThrowError("Extending Pivot Failed From serial");
    }
    CurrentBLEController.WriteSeatAngleCharacteristic(std::to_string(SeatAngleEncoder.getCount()));
    break;
  }

  case SeatControlExtendRequest:
  {
    SetCurrentOutgoingRequest(ReceivedCommandRequest);
    StopAllActuatorsMovingToSpecificLocation(); // Cancel any calls to Move actuators to specific locations
    Serial.println("Extend Seat Command");
    bWantsHome = false;
    bool SeatCanExtend = CurrentMotorController.ExtendActuator(&SeatExtensionActuator, &CurrentDeviceState);
    if (SeatCanExtend)
    {
      Serial.println("Extending Seat from serial");
    }
    else
    {
      ThrowError("Extending Seat Failed From serial");
    }
    CurrentBLEController.WriteSeatExtensionPositionCharacteristic(std::to_string(MidBackEncoder.getCount()));
    break;
  }
  case MidBackControlExtendRequest:
  {
    SetCurrentOutgoingRequest(ReceivedCommandRequest);
    StopAllActuatorsMovingToSpecificLocation(); // Cancel any calls to Move actuators to specific locations
    Serial.println("Extend Midback Command");
    bWantsHome = false;
    bool MidBackCanExtend = CurrentMotorController.ExtendActuator(&MidBackActuator, &CurrentDeviceState);
    if (MidBackCanExtend)
    {
      Serial.println("Extending MidBack from serial");
    }
    else
    {
      ThrowError("Extending Midback Failed From serial");
    }
    CurrentBLEController.WriteMidbackPositionCharacteristic(std::to_string(MidBackEncoder.getCount()));
    break;
  }
  case PivotControlRetractRequest:
  {
    SetCurrentOutgoingRequest(ReceivedCommandRequest);
    StopAllActuatorsMovingToSpecificLocation(); // Cancel any calls to Move actuators to specific locations
    Serial.println("Retract Pivot Command");
    bool PivotCanRetract = CurrentMotorController.RetractActuator(&PivotActuator, &CurrentDeviceState);
    bWantsHome = false;
    if (PivotCanRetract)
    {
      Serial.println("Retracting Pivot from serial");
    }
    else
    {
      ThrowError("Retracting Pivot Failed From serial");
    }
    CurrentBLEController.WriteSeatAngleCharacteristic(std::to_string(SeatAngleEncoder.getCount()));
    break;
  }
  case SeatControlRetractRequest:
  {
    SetCurrentOutgoingRequest(ReceivedCommandRequest);
    StopAllActuatorsMovingToSpecificLocation(); // Cancel any calls to Move actuators to specific locations
    Serial.println("Retract Seat Command");
    bool SeatCanRetract = CurrentMotorController.RetractActuator(&SeatExtensionActuator, &CurrentDeviceState);
    bWantsHome = false;
    if (SeatCanRetract)
    {
      Serial.println("Retracting Seat from serial");
    }
    else
    {
      ThrowError("Retracting Seat Failed From serial");
    }
    CurrentBLEController.WriteSeatExtensionPositionCharacteristic(std::to_string(MidBackEncoder.getCount()));
    break;
  }
  case MidBackControlRetractRequest:
  {
    SetCurrentOutgoingRequest(ReceivedCommandRequest);
    StopAllActuatorsMovingToSpecificLocation(); // Cancel any calls to Move actuators to specific locations
    Serial.println("Retract MB Command");
    bWantsHome = false;
    bool MidBackCanRetract = CurrentMotorController.RetractActuator(&MidBackActuator, &CurrentDeviceState);
    if (MidBackCanRetract)
    {
      Serial.println("Retracting MidBack from serial");
    }
    else
    {
      ThrowError("Retracting Midback Failed From serial");
    }
    CurrentBLEController.WriteMidbackPositionCharacteristic(std::to_string(MidBackEncoder.getCount()));
    CurrentState = ReceivedCommandRequest;
    break;
  }
  case MidBackGoToPositionRequest:
  {
    SetCurrentOutgoingRequest(ReceivedCommandRequest);
    bWantsHome = false;
    StopAllActuatorsMovingToSpecificLocation(); // Cancel any calls to Move actuators to specific locations
    CurrentBLEController.WriteMidbackPositionCharacteristic(std::to_string(MidBackEncoder.getCount()));
    CurrentDeviceState.bMidBackIsgoingToLocation = true; // set this flag so that the device will start going to a position
    Serial.println("Tryin to go to position MidBack");
    break;
  }

  case PivotControlGoToPositionRequest:
  {
    SetCurrentOutgoingRequest(ReceivedCommandRequest);
    bWantsHome = false;
    StopAllActuatorsMovingToSpecificLocation(); // Cancel any calls to Move actuators to specific locations
    CurrentBLEController.WriteSeatAngleCharacteristic(std::to_string(SeatAngleEncoder.getCount()));
    CurrentDeviceState.bMainPivotIsGoingToLocation = true; // set this flag so that the device will start going to a position
    Serial.println("Tryin to go to position Main Pivot");
    break;
  }

  case SeatExtensionGoToPositionRequest:
  {
    SetCurrentOutgoingRequest(ReceivedCommandRequest);
    bWantsHome = false;
    StopAllActuatorsMovingToSpecificLocation(); // Cancel any calls to Move actuators to specific locations
    CurrentBLEController.WriteSeatExtensionPositionCharacteristic(std::to_string(MidBackEncoder.getCount()));
    CurrentDeviceState.bSeatExtensionIsGoingToLocation = true; // set this flag so that the device will start going to a position
    Serial.println("Tryin to go to position Seat Extension");
    break;
  }

  case HomeRequest:
  {
    SetCurrentOutgoingRequest(ReceivedCommandRequest);
    StopAllActuatorsMovingToSpecificLocation(); // Cancel any calls to Move actuators to specific locations
    CurrentBLEController.WriteMidbackPositionCharacteristic(std::to_string(MidBackEncoder.getCount()));
    CurrentBLEController.WriteSeatAngleCharacteristic(std::to_string(SeatAngleEncoder.getCount()));
    CurrentBLEController.WriteSeatExtensionPositionCharacteristic(std::to_string(MidBackEncoder.getCount()));
    Serial.println("Wants Home");
    bWantsHome = true;
    break;
  }
  case StateRequest:
  {
    // this is just for debugging over serial and will not send this data over BLE
    // BLE Connections are limited to the information in the characteristics
    Serial.println("/////////////////");
    Serial.println("DEVICE STATE:");
    Serial.println("Seat Limit Active: ");
    Serial.print(CurrentDeviceState.bSeatLimitActive);
    Serial.println("");
    Serial.println("Seat Extension Limit Active: ");
    Serial.print(CurrentDeviceState.bSeatExtensionLimitActive);
    Serial.println("");
    Serial.println("Back Limit Active: ");
    Serial.print(CurrentDeviceState.bBackLimitActive);
    Serial.println("MidBack Limit Active: ");
    Serial.print(CurrentDeviceState.bMidBackLimitActive);
    Serial.println("");
    Serial.println("Main Pivot Reading: ");
    Serial.print(CurrentDeviceState.MainPivotAngle);
    Serial.println("");
    Serial.println("Seat Extension Amount: ");
    Serial.print(CurrentDeviceState.SeatExtension);
    Serial.println("");
    Serial.println("MidBack Extension Amount: ");
    Serial.print(CurrentDeviceState.MidBackExtension);
    Serial.println("");
    Serial.println("/////////////////");
    Serial.println("ACTUATOR STATES: ");
    Serial.println("");
    break;
  }
  case UpdatePositions:
  {
    CurrentBLEController.WriteMidbackPositionCharacteristic(std::to_string(MidBackEncoder.getCount()));
    CurrentBLEController.WriteSeatAngleCharacteristic(std::to_string(SeatAngleEncoder.getCount()));
    CurrentBLEController.WriteSeatExtensionPositionCharacteristic(std::to_string(MidBackEncoder.getCount()));
    break;
  }
  default:
    bWantsHome = false;
    ThrowError("INVALID COMMAND STOPPING MOVEMENT");
    Serial.println("Valid Commands:");
    Serial.println("Stop Pivot:  ");
    Serial.print(PivotControlStopRequest);
    Serial.println("");
    Serial.println("Stop Seat Extension:  ");
    Serial.print(SeatControlStopRequest);
    Serial.println("");
    Serial.println("Stop Midback control Extension: ");
    Serial.print(MidBackControlStopRequest);
    Serial.println("");
    Serial.println("Extend Pivot: ");
    Serial.print(PivotControlExtendRequest);
    Serial.println("");
    Serial.println("Extend Seat Extension: ");
    Serial.print(SeatControlExtendRequest);
    Serial.println("");
    Serial.println("Extend Midback control Extension: ");
    Serial.print(MidBackControlStopRequest);
    Serial.println("");
    Serial.println("Retract Pivot: ");
    Serial.print(PivotControlRetractRequest);
    Serial.println("");
    Serial.println("Retract Seat Extension: ");
    Serial.print(SeatControlRetractRequest);
    Serial.println("");
    Serial.println("Retract Midback control Extension: ");
    Serial.print(MidBackControlRetractRequest);
    Serial.println("");
    Serial.println("STOP ALL MOTION: ");
    Serial.print(StopAllMovementRequest);
    Serial.println("");
    Serial.println("RETURN TO SEATED: ");
    Serial.print(HomeRequest);
    Serial.println("");
    Serial.println("Print Device state");
    Serial.print(StateRequest);
    Serial.println("");
    break;
  }

  CurrentRequest = NoCommand;

#pragma endregion
}

void UpdateDevicestate()
{
  // get the values of all the limits from the interrupt flags below
  CurrentDeviceState.bSeatExtensionLimitActive = isSeatLimitActive;
  CurrentDeviceState.bSeatLimitActive = isSeatLimitActive;
  CurrentDeviceState.bBackLimitActive = isBackLimitActive;
  CurrentDeviceState.bMidBackLimitActive = isMidBackLimitActive;

  // read the current values of the encoders and copy them into the device state
  CurrentDeviceState.MidBackExtension = MidBackEncoder.getCount();
  CurrentDeviceState.MainPivotAngle = SeatAngleEncoder.getCount();
  CurrentDeviceState.SeatExtension = SeatExtensionEncoder.getCount();

  if (isSeatLimitActive)
  {
    SeatAngleEncoder.clearCount(); // the seat angle is at 0 point to reset the encoder to 0 in case it is a little off
  }
  if (isSeatExtLimitActive)
  {
    SeatExtensionEncoder.clearCount(); // the seat Extension is at 0 point to reset the encoder to 0 in case it is a little off
  }
  if(isMidBackLimitActive)
  {
    MidBackEncoder.clearCount();// the Midback is at 0 point to reset the encoder to 0 in case it is a little off
  }

  if ((!CurrentDeviceState.bBackLimitActive || CurrentDeviceState.bSeatLimitActive) && SeatExtensionActuator.ActuatorState == ExtendingState)
  {
    CurrentMotorController.StopActuator(&SeatExtensionActuator); // prevent the seat extension actuator from extending if we don't have the back supine
  }
  if ((!CurrentDeviceState.bSeatExtensionLimitActive && !CurrentDeviceState.bBackLimitActive) && PivotActuator.ActuatorState == RetractingState)
  {
    CurrentMotorController.StopActuator(&PivotActuator); // prevent the pivot from retracting to seated position if the seat is not fully retracted or the back is not supine
  }
  if (CurrentDeviceState.bBackLimitActive && CurrentDeviceState.bSeatLimitActive)
  {
    ThrowError("ERROR BACK AND SEAT LIMIT READING CONFLICT");
  }
}

void ThrowError(String ErrorString)
{
  // currently error will stop motion may consider changing this to homing request or implementing some sort of
  // emergency home to get patient seated when limit switches are non functional
  Serial.println(ErrorString);
  CurrentMotorController.StopAllMotion();
  SetCurrentOutgoingRequest(InternalErrorRequest);
}

void StopAllActuatorsMovingToSpecificLocation()
{
  if (CurrentDeviceState.bMainPivotIsGoingToLocation)
  {
    Serial.println("Canceling Main pivot go to location");
    CurrentMotorController.StopActuator(&PivotActuator);
    CurrentDeviceState.bMainPivotIsGoingToLocation = false;
    if (CurrentDeviceState.MidBackExtension == CurrentDeviceState.CurrentMidBackGoal)
    {
      SetCurrentOutgoingRequest(SucceededToReachSeatAngleGoalLocationRequest);
    }
    else
    {
      SetCurrentOutgoingRequest(FailedToReachSeatAngleGoalLocationRequest);
    }
  }
  if (CurrentDeviceState.bSeatExtensionIsGoingToLocation)
  {
    Serial.println("Canceling Seat Extension go to location");
    CurrentMotorController.StopActuator(&SeatExtensionActuator);
    CurrentDeviceState.bSeatExtensionIsGoingToLocation = false;
    if (CurrentDeviceState.SeatExtension == CurrentDeviceState.CurrentSeatExtensionGoal)
    {
      SetCurrentOutgoingRequest(SucceededToReachSeatExtensionGoalLocationRequest);
    }
    else
    {
      SetCurrentOutgoingRequest(FailedToReachSeatExtensionGoalLocationRequest);
    }
  }
  if (CurrentDeviceState.bMidBackIsgoingToLocation)
  {
    Serial.println("Canceling Mid Back Go to location");
    CurrentMotorController.StopActuator(&MidBackActuator);
    CurrentDeviceState.bMidBackIsgoingToLocation = false;
    if (CurrentDeviceState.MidBackExtension == CurrentDeviceState.CurrentMidBackGoal)
    {
      SetCurrentOutgoingRequest(SucceededToReachMidBackGoalLocationRequest);
    }
    else
    {
      SetCurrentOutgoingRequest(SucceededToReachMidBackGoalLocationRequest);
    }
  }
}

void SetCurrentOutgoingRequest(int NewRequest)
{
  if(NewRequest != CurrentRequest)
  {
    CurrentBLEController.WriteOutgoingCommsCharacteristic(std::to_string(NewRequest));
    CurrentState = NewRequest;
  }
  
}