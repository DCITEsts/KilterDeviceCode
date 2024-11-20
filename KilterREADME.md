KILTER TILTER DESCRIPTION

GENERAL ARCHITECTURE:

This program is written for the ESP32 using the arduino framework.

The bulk of the processing is done in setup and Loop in main.cpp.  The Loop function Essentially Updates the device internal state, Completes any commands then consumes any new commands and updates the device state on BLE in a loop. Some functions are pushed off into their respective classes.  BLEController handles all of the bluetooth setup and functionality and MotorController controls the motors and insures they are blocked from completing damaging operations.  GlobalPins.h contains most of the global variables and definitions pertaining to device state.  InterruptHandling.h holds key definitions for switch interrupts to work properly.

BLE COMMUNICATION GENERAL:

This device uses the BLE protocol.  The library is NimBLE. Important definitions are in BLEController.h.

DeviceName is the name of the device that will be advertised
The UUID's for Characterstics and Services are defined below DeviceName in BLEController.h.  Device name and UUID's can be changed if desired.

The CommsService handles communication of commands and device states.  CommsOutgoingCharacteristic contains the current device state written by the device.  CommsOutgoingCharacteristic should not be written to. CommsIncomingCharacteristic Is where commands should be written to be consumed by the device.

Valid commands you can send the unit are defined in GlobalPins.h in the region BLEGlobals.  They Consist of unique two digit integers each.  All commands written to the CommsIncomingCharacteristic must be integer values and unique per command.

Valid device states are found directly below the valid commands.  these are also integer values.  They do not technically have to be integers but the way that they are currently written to the device it's easier to keep them that way.  It would require minor changes to the way they are written to allow non integer values if needed.

To read encoder positions you will read from one of the three characteristics in the PositionService  that do not have request in their names (MidBackPositionCharacteristic = Midback Actuator Position, SeatAngleCharacteristic = Angle of seat, SeatExtensionCharacteristic = Extension of seat from back).  These integer values will correspond to the number of steps the encoder is away from it's zero point.  Do not attempt to write to these characteristics.

All characteristics are currently set write allowed to make debugging a little easier.  Things written to characteristics that aren't intended to be written to will be ignored.  In future releases the characteristics called out in this guide as not to be written will likely be set as read only so avoid writing to these characteristics to prevent future compatability issues.

When requesting an actuator position you will write to the corresponding Postion Characteristic with request in its name

COMMAND GENERAL DESCRIPTONS (CommsIncomingRequests):

The Commands are listed with a descriptive name in globalpins.h.  Commands with extend actuator extend the corresponding actuator and vice versa for retract actuator.  HomeRequest will bring the device back to a fully seated position (does not effect midback actuator position).  staterequest is really only for debugging use and will not do anything over the BLE Connection.  NoCommand does not need to be written explicitly.  The device will set the command to no command automatically once the most recent command is consumed.  Reading from the CommsIncomingRequestcharacteristic will not give a valid device state since requests could have been out of date or failed.

Device STATE GENERAL DESCRIPTION (CommsOutGoingRequests):

Possible device states are listed below the the commands in GlobalPins.h.  State is written to CommsOutgoinRequestscharacteristic.  These give the device current state updates.  IdleRequest is for when no commands have been received. IdleRequest does not mean that the device is not doing anything (Going to a positon or moving) it only means no command was received last update.  ReceivedCommand will be writen once a Command is received and processed.  Internal Error says that your command could not complete.  This is usually due to the actuators simply not being in a valid position to complete the command.  The FailedToReach and SucceededToReach states will notify when an actuator has stopped trying to move to a location and whether it was successful or not.

ACTUATOR CONTROLS:

Actuators are locked from performing moves that would damage the equipment.  Only one actuator can currently be moved at a time to prevent power consumption problems so subsequent calls to move multiple actuators without calls to explicitly stop actuators will simply cancel prior calls for movement.

Actuators that are not set to go to a specific position should be explicitly stopped.  Actuators will just extend/retract until they hit their internal limit switch, until the MotorController determines that they are not currently able to be moved or until a command to stop that actuator/all actuators is received.

ENCODER POSITIONS:

Encoder positions are not continuously updated in BLE to save on bandwidth.  Positions are only updated when a command is received that effects the actuator movement, when an actuator completes moving to a position, when an actuator fails to move to a position or when the Command UpdatePositions (77) is sent.  Encoder positions are updated continuously internally on the embedded device.

Encoders are zeroed automatically when the actuator for that encoder is in the fully retracted position.  If any actuator is extended when device is initially powered on then the 0 position for that actuator's encoder will be  incorrect leading to incorrect encoder readings for that actuator.  To zero an out of sync encoder you just have to fully retract the actuator t
hat encoder is associated with.

MOVING AN ACTUATOR TO A SPECIFIC POSITION:

Commands 13 (Seat angle go to), 23(Seat extension go to) and 33(Midback extension go to) request that each actuator go to the current position written to it's corresponding Position Request Characteristic in the Position Service.  While the actuator is moving to location the actuator will continuously update it's position and then write it's corresponding success code to the CommsOutgoingRequestCharacteristic once it either succeeds or fails to reach a location.

Actuators can fail to reach their position either because the actuator is currently unable to move to that position (determined by motor controller) or because it was interrupted by a conflicting command.  Any new, valid command sent during the movement except 66, 77 or 0 will cause the actuator to cancel move to position command and report either success or failure.

Only one actuator can be moving at a time and calls to move multiple actuators will cancel movement of all others.

Keep in mind that actuators will attempt to move to the location that was written to their corresponding position request characteristic at the time the go to command was received so write the correct value to the actuator's corresponding position request characteristic and check that the value was actually received first then send the go to postion command.  There is no range check on this value so if you request an out of range value it will just keep trying to go there until you either send another command or it fails for some external reason.