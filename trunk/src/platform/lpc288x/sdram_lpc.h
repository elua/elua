#ifndef __SDRAM_LPC
#define __SDRAM_LPC

#include "type.h"

#define SDRAM_PERIOD          16.6  // 60MHz
#define P2C(Period)           (((Period<SDRAM_PERIOD)?0:(u32)((float)Period/SDRAM_PERIOD))+1)

#define SDRAM_REFRESH         15625
#define SDRAM_TRP             20
#define SDRAM_TRAS            45
#define SDRAM_TAPR            1
#define SDRAM_TDAL            2
#define SDRAM_TWR             3
#define SDRAM_TRC             65
#define SDRAM_TRFC            66
#define SDRAM_TXSR            67
#define SDRAM_TRRD            15
#define SDRAM_TMRD            3

#define FLASH_TAC             120


#endif

