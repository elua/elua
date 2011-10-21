// eLua platform configuration

#ifndef __PLATFORM_CONF_H__
#define __PLATFORM_CONF_H__

#include "auxmods.h"
#include "stacks.h"

// *****************************************************************************
// Define here what components you want for this platform

#define BUILD_XMODEM
#define BUILD_SHELL
#define BUILD_ROMFS
#define BUILD_TERM
#define BUILD_CON_GENERIC
//#define BUILD_RPC

// *****************************************************************************
// UART/Timer IDs configuration data (used in main.c)

#define CON_UART_ID           1
#define CON_UART_SPEED        38400
#define CON_TIMER_ID          0
#define TERM_LINES            25
#define TERM_COLS             80

// *****************************************************************************
// Auxiliary libraries that will be compiled for this platform

#if defined( ELUA_BOOT_RPC ) && !defined( BUILD_RPC )
#define BUILD_RPC
#endif

#if defined( BUILD_RPC ) 
#define RPCLINE _ROM( AUXLIB_RPC, luaopen_rpc, rpc_map )
#else
#define RPCLINE
#endif

#define LUA_PLATFORM_LIBS_ROM\
  _ROM( AUXLIB_PIO, luaopen_pio, pio_map )\
  _ROM( AUXLIB_TMR, luaopen_tmr, tmr_map )\
  _ROM( AUXLIB_PD, luaopen_pd, pd_map )\
  _ROM( AUXLIB_PWM, luaopen_pwm, pwm_map )\
  _ROM( AUXLIB_UART, luaopen_uart, uart_map )\
  _ROM( AUXLIB_TERM, luaopen_term, term_map )\
  _ROM( AUXLIB_PACK, luaopen_pack, pack_map )\
  _ROM( AUXLIB_BIT, luaopen_bit, bit_map )\
  _ROM( AUXLIB_ELUA, luaopen_elua, elua_map )\
  _ROM( AUXLIB_CPU, luaopen_cpu, cpu_map )\
  RPCLINE\
  _ROM( LUA_MATHLIBNAME, luaopen_math, math_map )

// *****************************************************************************
// Configuration data

// Virtual timers (0 if not used)
#define VTMR_NUM_TIMERS       0
#define VTMR_FREQ_HZ          4

// Number of resources (0 if not available/not implemented)
#define NUM_PIO               2
#define NUM_SPI               0
#define NUM_UART              4
#define NUM_TIMER             4
#define NUM_PWM               3
#define NUM_ADC               0
#define NUM_CAN               0

// RPC boot options
#define RPC_UART_ID           CON_UART_ID
#define RPC_TIMER_ID          CON_TIMER_ID
#define RPC_UART_SPEED        CON_UART_SPEED

// CPU frequency (needed by the CPU module and MMCFS code, 0 if not used)
#define CPU_FREQUENCY         0

// PIO prefix ('0' for P0, P1, ... or 'A' for PA, PB, ...)
#define PIO_PREFIX            '0'
// Pins per port configuration:
// #define PIO_PINS_PER_PORT (n) if each port has the same number of pins, or
// #define PIO_PIN_ARRAY { n1, n2, ... } to define pins per port in an array
// Use #define PIO_PINS_PER_PORT 0 if this isn't needed
#define PIO_PINS_PER_PORT     16

// Allocator data: define your free memory zones here in two arrays
// (start address and end address)
#define SRAM_ORIGIN           0x20000000
#define SRAM_SIZE             0x10000
#define MEM_START_ADDRESS     { ( void* )end }
#define MEM_END_ADDRESS       { ( void* )( SRAM_ORIGIN + SRAM_SIZE - STACK_SIZE_TOTAL - 1 ) }

#endif // #ifndef __PLATFORM_CONF_H__
