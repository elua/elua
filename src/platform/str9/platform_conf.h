// eLua platform configuration

#ifndef __PLATFORM_CONF_H__
#define __PLATFORM_CONF_H__

#include "auxmods.h"
#include "stacks.h"
#include "type.h"
#include "elua_int.h"
#include "buf.h"

// *****************************************************************************
// Define here what components you want for this platform

#define BUILD_XMODEM
#define BUILD_SHELL
#define BUILD_ROMFS
#define BUILD_TERM
#define BUILD_CON_GENERIC
#define BUILD_ADC
//#define BUILD_RPC
#define BUILD_LUA_INT_HANDLERS
#define BUILD_C_INT_HANDLERS

#define PLATFORM_HAS_SYSTIMER

// *****************************************************************************
// UART/Timer IDs configuration data (used in main.c)

#ifdef ELUA_BOARD_STRE912
#define CON_UART_ID           0
#else // STR9-comStick
#define CON_UART_ID           1
#endif

#define CON_UART_SPEED        115200
#define TERM_LINES            25
#define TERM_COLS             80

// *****************************************************************************
// Configuration data

// Virtual timers (0 if not used)
#define VTMR_NUM_TIMERS       4
#define VTMR_FREQ_HZ          16
#define VTMR_TIMER_ID         3

// Number of resources (0 if not available/not implemented)
#define NUM_PIO               10
#define NUM_SPI               2
#define NUM_UART              3
#define NUM_PWM               4
#define NUM_ADC               8
#define NUM_CAN               0
#define NUM_I2C               2

// One timer is reserved for the system timer / vtimers
#define NUM_TIMER             3
#define NUM_PHYS_TIMER        4

// ADC Configuration Params
#define ADC_BIT_RESOLUTION    10
#define BUF_ENABLE_ADC
#define ADC_BUF_SIZE          BUF_SIZE_2

// These should be adjusted to support multiple ADC devices
#define ADC_TIMER_FIRST_ID    0
#define ADC_NUM_TIMERS        0

// RPC boot options
#define RPC_UART_ID           CON_UART_ID
#define RPC_UART_SPEED        CON_UART_SPEED

// Interrupt queue configuration
#define PLATFORM_INT_QUEUE_LOG_SIZE   BUF_SIZE_32

// CPU frequency (needed by the CPU module and MMCFS code, 0 if not used)
u32 SCU_GetMCLKFreqValue();
#define CPU_FREQUENCY         ( SCU_GetMCLKFreqValue() * 1000 )

// PIO prefix ('0' for P0, P1, ... or 'A' for PA, PB, ...)
#define PIO_PREFIX            '0'
// Pins per port configuration:
// #define PIO_PINS_PER_PORT (n) if each port has the same number of pins, or
// #define PIO_PIN_ARRAY { n1, n2, ... } to define pins per port in an array
// Use #define PIO_PINS_PER_PORT 0 if this isn't needed
#define PIO_PINS_PER_PORT     8

// Allocator data: define your free memory zones here in two arrays
// (start address and end address)
#define SRAM_ORIGIN           0x40000000
#define SRAM_SIZE             0x18000
#define MEM_START_ADDRESS     { ( void* )end }
#define MEM_END_ADDRESS       { ( void* )( SRAM_ORIGIN + SRAM_SIZE - STACK_SIZE_TOTAL - 1 ) }

// *****************************************************************************
// Auxiliary libraries that will be compiled for this platform

// The name of the platform specific libs table
#define PS_LIB_TABLE_NAME   "str9"

#ifdef BUILD_ADC
#define ADCLINE _ROM( AUXLIB_ADC, luaopen_adc, adc_map )
#else
#define ADCLINE
#endif

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
  ADCLINE\
  _ROM( AUXLIB_PD, luaopen_pd, pd_map )\
  _ROM( AUXLIB_UART, luaopen_uart, uart_map )\
  _ROM( AUXLIB_TERM, luaopen_term, term_map )\
  _ROM( AUXLIB_PACK, luaopen_pack, pack_map )\
  _ROM( AUXLIB_BIT, luaopen_bit, bit_map )\
  _ROM( AUXLIB_CPU, luaopen_cpu, cpu_map)\
  _ROM( AUXLIB_CPU, luaopen_elua, elua_map)\
  _ROM( AUXLIB_I2C, luaopen_i2c, i2c_map)\
  _ROM( AUXLIB_SPI, luaopen_spi, spi_map)\
  _ROM( AUXLIB_ELUA, luaopen_elua, elua_map)\
  RPCLINE\
  _ROM( AUXLIB_PWM, luaopen_pwm, pwm_map)\
  _ROM( LUA_MATHLIBNAME, luaopen_math, math_map )\
  _ROM( PS_LIB_TABLE_NAME, luaopen_platform, platform_map )

 // Interrupt list
#define INT_GPIO_POSEDGE      ELUA_INT_FIRST_ID
#define INT_GPIO_NEGEDGE      ( ELUA_INT_FIRST_ID + 1 )
#define INT_TMR_MATCH         ( ELUA_INT_FIRST_ID + 2 )
#define INT_ELUA_LAST         INT_TMR_MATCH

#define PLATFORM_CPU_CONSTANTS\
 _C( INT_GPIO_POSEDGE ),\
 _C( INT_GPIO_NEGEDGE ),\
 _C( INT_TMR_MATCH )

#endif // #ifndef __PLATFORM_CONF_H__

