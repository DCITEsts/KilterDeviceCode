#include <Arduino.h>
#include "GlobalPins.h"
#include "MotorController.h"
#include "InterruptHandling.h"
#include "BLEController.h"
#include "strings.h"
#include "string.h"

// put function declarations here:
void UpdateDevicestate();

void ThrowError(String ErrorString);

static MotorController CurrentMotorController;
static BLEController CurrentBLEController;

bool bWantsHome = false;

void setup()
{

  Serial.begin(115200);

  
  Serial.println("Setting up");
  pinMode(SeatLimitSwitch, INPUT);
  pinMode(SeatExtensionLimitSwitch, INPUT);
  pinMode(BackLimitSwitch, INPUT);

  pinMode(PivotActuatorA, OUTPUT);
  pinMode(PivotActuatorB, OUTPUT);
  pinMode(SeatActuatorA, OUTPUT);
  pinMode(SeatActuatorB, OUTPUT);
  pinMode(MidBackActuatorA, OUTPUT);
  pinMode(MidBackActuatorB, OUTPUT);

  digitalWrite(PivotActuatorA, LOW);
  digitalWrite(PivotActuatorB, LOW);
  digitalWrite(SeatActuatorA, LOW);
  digitalWrite(SeatActuatorB, LOW);
  digitalWrite(MidBackActuatorA, LOW);
  digitalWrite(MidBackActuatorB, LOW);

  CurrentMotorController.StopAllMotion();

  isSeatLimitActive = (digitalRead(SeatLimitSwitch) == HIGH);
  isSeatExtLimitActive = (digitalRead(SeatExtensionLimitSwitch) == HIGH);
  isBackLimitActive = (digitalRead(BackLimitSwitch) == HIGH);

  CurrentBLEController.BLEsetup();

  UpdateDevicestate();

  attachInterrupt(digitalPinToInterrupt(SeatExtensionLimitSwitch), onSeatExtLimitActive, CHANGE);
  attachInterrupt(digitalPinToInterrupt(SeatLimitSwitch), onSeatLimitActive, CHANGE);
  attachInterrupt(digitalPinToInterrupt(BackLimitSwitch), onBackLimitActive, CHANGE);

  Serial.println("Setup Complete");
}

void loop()
{
  CurrentRequest = LatestReadValue;  //consume buffered command from ble
  LatestReadValue = NoCommand; //reset command buffer

  if (Serial.available() > 0)// if the serial has commands overwrite the BLE Command with the serial.  Priority given to serial commands if available
  {
    CurrentRequest = Serial.parseInt();
    Serial.println("Serial Commmand Received:");
    Serial.print(CurrentRequest);
    Serial.println("");
    Serial.flush();
    while(Serial.available() > 0)
    {
      Serial.read();
    }
  }

  UpdateDevicestate();

#pragma region HandleRequestsSwitch
  switch (CurrentRequest)
  {
  case NoCommand:
    // no commands received just don't do anything
    break;
  case StopAllMovementRequest:
    Serial.println("Stopping all movement Command");
    bWantsHome = false;
    CurrentMotorController.StopAllMotion();
    break;
  case PivotControlStopRequest:
    bWantsHome = false;
    Serial.println("Stop Pivot Command");
    CurrentMotorController.StopActuator(&PivotActuator);
    break;

  case SeatControlStopRequest:
    bWantsHome = false;
    Serial.println("Stop Seat Command");
    CurrentMotorController.StopActuator(&SeatExtensionActuator);
    break;

  case MidBackControlStopRequest:
    bWantsHome = false;
    Serial.println("Stop MB Command");
    CurrentMotorController.StopActuator(&MidBackActuator);
    break;

  case PivotControlExtendRequest:
  {
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
    break;
  }

  case SeatControlExtendRequest:
  {
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
    break;
  }
  case MidBackControlExtendRequest:
  {
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
    break;
  }
  case PivotControlRetractRequest:
  {
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
    break;
  }
  case SeatControlRetractRequest:
  {
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
    break;
  }
  case MidBackControlRetractRequest:
  {
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
    break;
  }
  case HomeRequest:
    Serial.println("Wants Home");
    bWantsHome = true;
    break;

  case StateRequest:
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

#pragma region HandleHoming

  if (bWantsHome && !CurrentDeviceState.bSeatExtensionLimitActive)
  {
    Serial.println("Homing Seat Extension");
    CurrentMotorController.RetractActuator(&SeatExtensionActuator, &CurrentDeviceState); // if the seat is still extended and we're trying to home retract the seat first
  }
  if (bWantsHome && CurrentDeviceState.bSeatExtensionLimitActive && !CurrentDeviceState.bSeatLimitActive)
  {
    Serial.println("Homing Pivot");
    CurrentMotorController.RetractActuator(&PivotActuator, &CurrentDeviceState); //if the seat is not extended and we want home then retract the pivot to get to a seated position
  }
  if (CurrentDeviceState.bSeatExtensionLimitActive && CurrentDeviceState.bSeatLimitActive && !CurrentDeviceState.bBackLimitActive)
  {
    bWantsHome = false; // we're home! give up on homing if we're trying to do it.
  }
#pragma endregion

  UpdateDevicestate();
}

void UpdateDevicestate()
{
  // get the values of all the limits from the interrupt flags below
  CurrentDeviceState.bSeatLimitActive = isSeatLimitActive;
  CurrentDeviceState.bSeatExtensionLimitActive = isSeatExtLimitActive;
  CurrentDeviceState.bBackLimitActive = isBackLimitActive;

  if ((!CurrentDeviceState.bBackLimitActive || CurrentDeviceState.bSeatLimitActive) && SeatExtensionActuator.ActuatorState == ExtendingState)
  {
    
    CurrentMotorController.StopActuator(&SeatExtensionActuator);  // prevent the seat extension actuator from extending if we don't have the back supine
  }
  if ((!CurrentDeviceState.bSeatExtensionLimitActive && !CurrentDeviceState.bBackLimitActive) && PivotActuator.ActuatorState == RetractingState)
  {
    
    CurrentMotorController.StopActuator(&PivotActuator); //prevent the pivot from retracting to seated position if the seat is not fully retracted or the back is not supine
  }
  if (CurrentDeviceState.bBackLimitActive && CurrentDeviceState.bSeatLimitActive)
  {
    ThrowError("ERROR BACK AND SEAT LIMIT READING CONFLICT");
  }
}

void ThrowError(String ErrorString)
{
  //currently error will stop motion may consider changing this to homing request or implementing some sort of 
  //emergency home to get patient seated when limit switches are non functional
  Serial.println(ErrorString);
  CurrentMotorController.StopAllMotion(); 
  CurrentRequest = InternalErrorRequest;

  
}