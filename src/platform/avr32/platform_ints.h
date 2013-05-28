// Interrupts for this platform

#ifndef __PLATFORM_INTS_H__
#define __PLATFORM_INTS_H__

#include "elua_int.h"

// Interrupt list
#define INT_UART_RX           ELUA_INT_FIRST_ID
#define INT_TMR_MATCH         ( ELUA_INT_FIRST_ID + 1 )
#define INT_GPIO_POSEDGE      ( ELUA_INT_FIRST_ID + 2 )
#define INT_GPIO_NEGEDGE      ( ELUA_INT_FIRST_ID + 3 )
#define INT_ELUA_LAST         INT_GPIO_NEGEDGE

#endif

