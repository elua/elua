// eLua platform configuration

#ifndef __PLATFORM_CONF_H__
#define __PLATFORM_CONF_H__

#include "auxmods.h"
#include "board.h"
#include "stacks.h"

// *****************************************************************************
// Define here what components you want for this platform

#define BUILD_XMODEM
#define BUILD_SHELL
#define BUILD_ROMFS
#define BUILD_TERM
#define BUILD_CON_GENERIC

// *****************************************************************************
// UART/Timer IDs configuration data (used in main.c)

#define CON_UART_ID         0
#define CON_UART_SPEED      38400
#define XMODEM_TIMER_ID     0
#define TERM_TIMER_ID       0
#define TERM_LINES          25
#define TERM_COLS           80
#define TERM_TIMEOUT        100000

// *****************************************************************************
// Auxiliary libraries that will be compiled for this platform

#define LUA_PLATFORM_LIBS\
  { AUXLIB_PD, luaopen_pd },\
  { AUXLIB_UART, luaopen_uart },\
  { AUXLIB_PIO, luaopen_pio },\
  { AUXLIB_TMR, luaopen_tmr },\
  { AUXLIB_TERM, luaopen_term },\
  { AUXLIB_PACK, luaopen_pack },\
  { AUXLIB_BIT, luaopen_bit },\
  { AUXLIB_CPU, luaopen_cpu },\
  { LUA_MATHLIBNAME, luaopen_math }

// *****************************************************************************
// Configuration data

// Virtual timers (0 if not used)
#define VTMR_NUM_TIMERS       4
#define VTMR_FREQ_HZ          4

// Number of resources (0 if not available/not implemented)
#define NUM_PIO               5
#define NUM_SPI               0
#define NUM_UART              4
#if VTMR_NUM_TIMERS > 0
#define NUM_TIMER             2
#else
#define NUM_TIMER             3
#endif      
#define NUM_PWM               0

// CPU frequency (needed by the CPU module, 0 if not used)
#define CPU_FREQUENCY         REQ_CPU_FREQ

// PIO prefix ('0' for P0, P1, ... or 'A' for PA, PB, ...)
#define PIO_PREFIX            'A'
// Pins per port configuration:
// #define PIO_PINS_PER_PORT (n) if each port has the same number of pins, or
// #define PIO_PIN_ARRAY { n1, n2, ... } to define pins per port in an array
// Use #define PIO_PINS_PER_PORT 0 if this isn't needed
#define PIO_PIN_ARRAY         { 31, 32, 6, 32, 8 }

// Allocator data: define your free memory zones here in two arrays
// (start address and end address)
#define MEM_START_ADDRESS     { ( void* )end }
#define MEM_END_ADDRESS       { ( void* )( 0x10000 - STACK_SIZE_TOTAL - 1 ) }

// *****************************************************************************
// CPU constants that should be exposed to the eLua "cpu" module

#endif // #ifndef __PLATFORM_CONF_H__
