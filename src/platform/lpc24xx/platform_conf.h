// eLua platform configuration

#ifndef __PLATFORM_CONF_H__
#define __PLATFORM_CONF_H__

#include "auxmods.h"
#include "stacks.h"
#include "target.h"

// *****************************************************************************
// Define here what components you want for this platform

#define BUILD_XMODEM
#define BUILD_SHELL
#define BUILD_ROMFS
#define BUILD_TERM
#define BUILD_CON_GENERIC

// *****************************************************************************
// UART/Timer IDs configuration data (used in main.c)

#define CON_UART_ID           0
#define CON_UART_SPEED        115200
#define CON_TIMER_ID          0
#define TERM_LINES            25
#define TERM_COLS             80

// *****************************************************************************
// Auxiliary libraries that will be compiled for this platform

#define LUA_PLATFORM_LIBS_ROM\
  _ROM( AUXLIB_PIO, luaopen_pio, pio_map )\
  _ROM( AUXLIB_TMR, luaopen_tmr, tmr_map )\
  _ROM( AUXLIB_UART, luaopen_uart, uart_map )\
  _ROM( AUXLIB_PIO, luaopen_pio, pio_map )\
  _ROM( AUXLIB_PD, luaopen_pd, pd_map )\
  _ROM( AUXLIB_TERM, luaopen_term, term_map )\
  _ROM( AUXLIB_PACK, luaopen_pack, pack_map )\
  _ROM( AUXLIB_BIT, luaopen_bit, bit_map )\
  _ROM( AUXLIB_CPU, luaopen_cpu, cpu_map )\
  _ROM( AUXLIB_PWM, luaopen_pwm, pwm_map )\
  _ROM( LUA_MATHLIBNAME, luaopen_math, math_map )

// *****************************************************************************
// Configuration data

// Virtual timers (0 if not used)
#define VTMR_NUM_TIMERS       4
#define VTMR_FREQ_HZ          4

// Number of resources (0 if not available/not implemented)
#define NUM_PIO               5
#define NUM_SPI               0
#define NUM_UART              4
#define NUM_PWM               12
#define NUM_ADC               0
#define NUM_CAN               0
// If virtual timers are enabled, the last timer will be used only for them
#if VTMR_NUM_TIMERS == 0
#define NUM_TIMER             4
#else
#define NUM_TIMER             3
#endif

// Enable RX buffering on UART
// [TODO] make this happen
//#define BUF_ENABLE_UART
//#define CON_BUF_SIZE          BUF_SIZE_128

// CPU frequency (needed by the CPU module, 0 if not used)
#define CPU_FREQUENCY         Fcclk

// PIO prefix ('0' for P0, P1, ... or 'A' for PA, PB, ...)
#define PIO_PREFIX            '0'
// Pins per port configuration:
// #define PIO_PINS_PER_PORT (n) if each port has the same number of pins, or
// #define PIO_PIN_ARRAY { n1, n2, ... } to define pins per port in an array
// Use #define PIO_PINS_PER_PORT 0 if this isn't needed
#define PIO_PINS_PER_PORT     31

// Allocator data: define your free memory zones here in two arrays
// (start address and end address)
#define SRAM_ORIGIN           0x40000000
#define SRAM_SIZE             0x10000 // [TODO]: make this 96k?
#define MEM_START_ADDRESS     { ( void* )end }
#define MEM_END_ADDRESS       { ( void* )( SRAM_ORIGIN + SRAM_SIZE - STACK_SIZE_TOTAL - 1 ) }

// *****************************************************************************
// CPU constants that should be exposed to the eLua "cpu" module

#define PINSEL_BASE_ADDR	0xE002C000
#define IO_PINSEL0           ( PINSEL_BASE_ADDR + 0x00 )
#define IO_PINSEL1           ( PINSEL_BASE_ADDR + 0x04 )
#define IO_PINSEL2           ( PINSEL_BASE_ADDR + 0x08 )
#define IO_PINSEL3           ( PINSEL_BASE_ADDR + 0x0C )
#define IO_PINSEL4           ( PINSEL_BASE_ADDR + 0x10 )
#define IO_PINSEL5           ( PINSEL_BASE_ADDR + 0x14 )
#define IO_PINSEL6           ( PINSEL_BASE_ADDR + 0x18 )
#define IO_PINSEL7           ( PINSEL_BASE_ADDR + 0x1C )
#define IO_PINSEL8           ( PINSEL_BASE_ADDR + 0x20 )
#define IO_PINSEL9           ( PINSEL_BASE_ADDR + 0x24 )
#define IO_PINSEL10          ( PINSEL_BASE_ADDR + 0x28 )

#define PLATFORM_CPU_CONSTANTS\
 _C( IO_PINSEL0 ),\
 _C( IO_PINSEL1 ),\
 _C( IO_PINSEL2 ),\
 _C( IO_PINSEL3 ),\
 _C( IO_PINSEL4 ),\
 _C( IO_PINSEL5 ),\
 _C( IO_PINSEL6 ),\
 _C( IO_PINSEL7 ),\
 _C( IO_PINSEL8 ),\
 _C( IO_PINSEL9 ),\
 _C( IO_PINSEL10 )
 
#endif // #ifndef __PLATFORM_CONF_H__

