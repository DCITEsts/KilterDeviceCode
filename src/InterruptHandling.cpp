#include "InterruptHandling.h"

volatile DRAM_ATTR bool isSeatLimitActive = false;
volatile DRAM_ATTR bool isSeatExtLimitActive = false;
volatile DRAM_ATTR bool isBackLimitActive = false;
volatile DRAM_ATTR bool isMidBackLimitActive = false;