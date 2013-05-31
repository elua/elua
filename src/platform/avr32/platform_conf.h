// eLua platform configuration

#ifndef __PLATFORM_CONF_H__
#define __PLATFORM_CONF_H__

#include "auxmods.h"
#include "board.h"
#include "stacks.h"
#include "elua_int.h"

#define PLATFORM_HAS_SYSTIMER

#if BOARD == EVK1100
    #include "EVK1100/evk1100_conf.h"
#elif BOARD == EVK1101
    #include "EVK1101/evk1101_conf.h"
#elif BOARD == MIZAR32
    #include "MIZAR32/mizar32_conf.h"
#else
    #error No known AVR32 board defined    
#endif

// Common parts

// Interrupt queue size
#define PLATFORM_INT_QUEUE_LOG_SIZE 5

// Interrupt list
#define INT_UART_RX           ELUA_INT_FIRST_ID
#define INT_TMR_MATCH         ( ELUA_INT_FIRST_ID + 1 )
#define INT_GPIO_POSEDGE      ( ELUA_INT_FIRST_ID + 2 )
#define INT_GPIO_NEGEDGE      ( ELUA_INT_FIRST_ID + 3 )
#define INT_ELUA_LAST         INT_GPIO_NEGEDGE

#define PLATFORM_CPU_CONSTANTS\
 _C( INT_UART_RX ),\
 _C( INT_TMR_MATCH ),\
 _C( INT_GPIO_POSEDGE ),\
 _C( INT_GPIO_NEGEDGE )

#endif // #ifndef __PLATFORM_CONF_H__
