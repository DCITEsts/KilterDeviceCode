#include "MotorController.h"
#include "GlobalPins.h"
#include <Arduino.h>

bool MotorController::ExtendActuator(ActuatorDefinition *Actuator, DeviceState *State)
{
    StopAllMotion();
    bool bCanExtendActuator = false;
    switch (Actuator->ActuatorID)
    {
    case PivotActID:
        bCanExtendActuator = CanExtendPivot(State);
        break;
    case SeatActID:
        bCanExtendActuator = CanExtendSeat(State);
        break;
    case MidBackActID:
        bCanExtendActuator = CanExtendMidback(State);
        break;
    default:
        bCanExtendActuator = false;
        Serial.println("Actuator passed to extend out of range");
        break;
    }

    if(bCanExtendActuator)
    {
        digitalWrite(Actuator->PinA, ExtendAct.InputPin1Value);
        digitalWrite(Actuator->PinB, ExtendAct.InputPin2Value);
        Actuator->ActuatorState = ExtendingState;
        return true;
    }
    else
    {
        StopAllMotion();
    }
    return false;
}

bool MotorController::RetractActuator(ActuatorDefinition *Actuator, DeviceState *State)
{
    StopAllMotion();
    bool bCanRetractActuator = false;
    switch (Actuator->ActuatorID)
    {
    case PivotActID:
        bCanRetractActuator = CanRetractPivot(State);
        break;
    case SeatActID:
        bCanRetractActuator = CanRetractSeat(State);
        break;
    case MidBackActID :
        bCanRetractActuator = CanRetractMidBack(State);
        break;
    default:
        bCanRetractActuator = false;
        Serial.println("Actuator passed to Retract out of range");
        break;
    }

    if(bCanRetractActuator)
    {
        digitalWrite(Actuator->PinA, RetractAct.InputPin1Value);
        digitalWrite(Actuator->PinB, RetractAct.InputPin2Value);
        Actuator->ActuatorState = RetractingState;
        return true;
    }
    return false;
}
bool MotorController::ActuatorTryGoToPosition(ActuatorDefinition *Actuator, DeviceState *State)
{
    int GoalLocation = 0;
    int CurrentLocation = 0;
    switch (Actuator->ActuatorID)
    {
    case PivotActID:
        CurrentLocation = State->MainPivotAngle;
        GoalLocation = State->CurrentMainPivotGoal;
        break;
    case SeatActID:
        CurrentLocation = State->SeatExtension;
        GoalLocation = State->CurrentSeatExtensionGoal;
        break;
    case MidBackActID:
        CurrentLocation = State->MidBackExtension;
        GoalLocation = State->CurrentMidBackGoal;
        break;
    default:
        break;
    }
    if(GoalLocation > CurrentLocation)
    {
      return ExtendActuator(Actuator, State);
    }
    else if(GoalLocation < CurrentLocation)
    {
      return RetractActuator(Actuator, State);
    }
    else
    {
      StopActuator(Actuator);
      return true;
    }
}

void MotorController::StopActuator(ActuatorDefinition *Actuator)
{
    digitalWrite(Actuator->PinA, StopAct.InputPin1Value);
    digitalWrite(Actuator->PinB, StopAct.InputPin2Value);
    Actuator->ActuatorState = StoppedState;
}

void MotorController::StopAllMotion()
{
    StopActuator(&PivotActuator);
    StopActuator(&SeatExtensionActuator);
    StopActuator(&MidBackActuator);
}


bool MotorController::CanExtendPivot(DeviceState *State)
{
    return true;
}

bool MotorController::CanRetractPivot(DeviceState *State)
{
    return (State->bBackLimitActive || State->bSeatExtensionLimitActive);
}

bool MotorController::CanExtendSeat(DeviceState *State)
{
    return (State->bBackLimitActive);
}

bool MotorController::CanRetractSeat(DeviceState *State)
{
    return true;
}

bool MotorController::CanExtendMidback(DeviceState *State)
{
    return true;
}

bool MotorController::CanRetractMidBack(DeviceState *State)
{
    return true;
}