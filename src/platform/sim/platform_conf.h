// eLua platform configuration

#ifndef __PLATFORM_CONF_H__
#define __PLATFORM_CONF_H__

#include "auxmods.h"
#include "type.h"
#include "stacks.h"
#include "buf.h"

// *****************************************************************************
// Define here what components you want for this platform

#define BUILD_SHELL
#define BUILD_ROMFS
#define BUILD_CON_GENERIC
#define BUILD_TERM
//#define BUILD_RFS
#define BUILD_WOFS

#define TERM_LINES    25
#define TERM_COLS     80

#define PLATFORM_HAS_SYSTIMER

// *****************************************************************************
// Auxiliary libraries that will be compiled for this platform

#define LUA_PLATFORM_LIBS_ROM\
  _ROM( AUXLIB_PD, luaopen_pd, pd_map )\
  _ROM( LUA_MATHLIBNAME, luaopen_math, math_map )\
  _ROM( AUXLIB_TERM, luaopen_term, term_map )\
  _ROM( AUXLIB_ELUA, luaopen_elua, elua_map )\
  _ROM( AUXLIB_TMR, luaopen_tmr, tmr_map )\

// Bogus defines for common.c
#define CON_UART_ID           0
#define CON_UART_SPEED        0

// *****************************************************************************
// Configuration data

// Virtual timers (0 if not used)
#define VTMR_NUM_TIMERS       0

// Number of resources (0 if not available/not implemented)
#define NUM_PIO               0
#define NUM_SPI               0
#define NUM_UART              0
#define NUM_TIMER             0
#define NUM_PWM               0
#define NUM_ADC               0
#define NUM_CAN               0

// CPU frequency (needed by the CPU module and MMCFS code, 0 if not used)
#define CPU_FREQUENCY         0

// PIO prefix ('0' for P0, P1, ... or 'A' for PA, PB, ...)
#define PIO_PREFIX            'A'
// Pins per port configuration:
// #define PIO_PINS_PER_PORT (n) if each port has the same number of pins, or
// #define PIO_PIN_ARRAY { n1, n2, ... } to define pins per port in an array
// Use #define PIO_PINS_PER_PORT 0 if this isn't needed
#define PIO_PINS_PER_PORT     0

// Allocator data: define your free memory zones here in two arrays
// (start address and end address)
extern void *memory_start_address;
extern void *memory_end_address;
#define MEM_LENGTH (1024 * 1024)
#define MEM_START_ADDRESS     { ( void* )memory_start_address }
#define MEM_END_ADDRESS       { ( void* )memory_end_address }

// RFS configuration
#define RFS_TIMEOUT           0 // dummy, always blocking by implementation
#define RFS_BUFFER_SIZE       BUF_SIZE_512

#endif // #ifndef __PLATFORM_CONF_H__
