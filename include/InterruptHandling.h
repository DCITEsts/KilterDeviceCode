#pragma once

#ifndef INTERRUPTHANDLING_H
#define INTERRUPTHANDLING_H

#include <Arduino.h>
#include "GlobalPins.h"

//internal interrupt values set by the interrupts to determine if a switch was pressed
//the way this works now we could just switch to chekcing these in main loop if desired
extern volatile DRAM_ATTR bool isSeatLimitActive;
extern volatile DRAM_ATTR bool isSeatExtLimitActive;
extern volatile DRAM_ATTR bool isBackLimitActive;
extern volatile DRAM_ATTR bool isMidBackLimitActive;


//Functions to interrupt

static void IRAM_ATTR onSeatLimitActive()

{
    isSeatLimitActive = !(digitalRead(SeatLimitSwitch) == HIGH);
}

static void IRAM_ATTR onSeatExtLimitActive()

{
    isSeatExtLimitActive = !(digitalRead(SeatExtensionLimitSwitch) == HIGH);
}

static void IRAM_ATTR onBackLimitActive()

{
    isBackLimitActive = !(digitalRead(BackLimitSwitch) == HIGH);
}

static void IRAM_ATTR onMidBackLimitActive()
{
    isMidBackLimitActive = (digitalRead(MidBackLimitSwitch) == HIGH);
}
#endif