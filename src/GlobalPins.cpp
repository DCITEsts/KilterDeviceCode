#include "GlobalPins.h"
#include <string>

struct ActuatorDefinition PivotActuator = {PivotActID, PivotActuatorA, PivotActuatorB, StoppedState};
struct ActuatorDefinition SeatExtensionActuator = {SeatActID, SeatActuatorA, SeatActuatorB, StoppedState};
struct ActuatorDefinition MidBackActuator = {MidBackActID, MidBackActuatorA, MidBackActuatorB, StoppedState};
struct DeviceState CurrentDeviceState = {false, false, false, 0, 0, 0};

int CurrentRequest = NoCommand;

std::string LatestUUIDWritten = "0";
int LatestReadValue = NoCommand;