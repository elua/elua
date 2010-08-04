// eLua platform configuration

#ifndef __PLATFORM_CONF_H__
#define __PLATFORM_CONF_H__

#include "auxmods.h"
#include "board.h"
#include "stacks.h"

#if BOARD == EVK1100
    #include "EVK1100/evk1100_conf.h"
#elif BOARD == EVK1101
    #include "EVK1101/evk1101_conf.h"
#else
    #error No known AVR32 board defined    
#endif

#endif // #ifndef __PLATFORM_CONF_H__
