// eLua platform configuration

#ifndef __PLATFORM_CONF_H__
#define __PLATFORM_CONF_H__

#include "auxmods.h"
#include "type.h"
#include "stacks.h"

#define BUILD_CON_GENERIC
#define BUILD_SHELL
#define BUILD_TERM
#define BUILD_ROMFS
#define BUILD_XMODEM
#define BUILD_LINENOISE

// *****************************************************************************
// UART/Timer IDs configuration data (used in main.c)

#define CON_UART_ID           0
#define CON_UART_SPEED        115200
#define TERM_LINES            25
#define TERM_COLS             80

#define PLATFORM_HAS_SYSTIMER

// *****************************************************************************
// Auxiliary libraries that will be compiled for this platform

#define LUA_PLATFORM_LIBS_ROM\
  _ROM( LUA_MATHLIBNAME, luaopen_math, math_map )\
  _ROM( AUXLIB_ELUA, luaopen_elua, elua_map )\
  _ROM( AUXLIB_TMR, luaopen_tmr, tmr_map )\
  _ROM( AUXLIB_PIO, luaopen_pio, pio_map )

// *****************************************************************************
// Configuration data

#define EGC_INITIAL_MODE      1

// Number of resources (0 if not available/not implemented)
#define NUM_PIO               16
#define NUM_SPI               0
#define NUM_UART              1
#define NUM_TIMER             0
#define NUM_PHYS_TIMER        0
#define NUM_PWM               0
#define NUM_ADC               0
#define NUM_CAN               0

// PIO prefix ('0' for P0, P1, ... or 'A' for PA, PB, ...)
#define PIO_PREFIX            '0'
// Pins per port configuration:
// #define PIO_PINS_PER_PORT (n) if each port has the same number of pins, or
// #define PIO_PIN_ARRAY { n1, n2, ... } to define pins per port in an array
// Use #define PIO_PINS_PER_PORT 0 if this isn't needed
#define PIO_PIN_ARRAY         { 16, 16, 16, 16, 8, 12, 7, 0, 0, 0, 0, 0, 0, 0, 14, 12 }

#define LINENOISE_HISTORY_SIZE_LUA    50
#define LINENOISE_HISTORY_SIZE_SHELL  10

// Interrupt queue size
#define PLATFORM_INT_QUEUE_LOG_SIZE 5

// Allocator data: define your free memory zones here in two arrays
// (start address and end address)
#define SRAM_SIZE             ( 64 * 1024 )
#define SRAM_BASE             0x20000000
#define MEM_START_ADDRESS     { ( void* )end }
#define MEM_END_ADDRESS       { ( void* )( SRAM_BASE + SRAM_SIZE - STACK_SIZE_TOTAL - 1 ) }

#define CPU_FREQUENCY         120000000

#endif // #ifndef __PLATFORM_CONF_H__

