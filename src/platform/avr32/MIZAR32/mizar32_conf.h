// eLua platform configuration

// Simplemachines.it Mizar32 board has 128Kbytes of flash with 8kb of bootloader
// To fit in 120K, build using:
// scons board=mizar32 target=lualong optram=0 allocator=newlib

#ifndef __MIZAR32_CONF_H__
#define __MIZAR32_CONF_H__

#include "sdramc.h"

// *****************************************************************************
// Define here what components you want for this platform

#define BUILD_MMCFS
//#define BUILD_XMODEM
//#define BUILD_SHELL
//#define BUILD_ROMFS
//#define BUILD_TERM
#define BUILD_CON_GENERIC
//#define BUILD_RPC

// *****************************************************************************
// UART/Timer IDs configuration data (used in main.c)

#define CON_UART_ID         0
#define CON_UART_SPEED      115200
#define CON_TIMER_ID        0
#define TERM_LINES          25
#define TERM_COLS           80

// *****************************************************************************
// SPI pins configuration data

#define BOARD_SPI0_SCK_PIN                  AVR32_PIN_PA13
#define BOARD_SPI0_SCK_PIN_FUNCTION         0
#define BOARD_SPI0_MISO_PIN                 AVR32_PIN_PA11
#define BOARD_SPI0_MISO_PIN_FUNCTION        0
#define BOARD_SPI0_MOSI_PIN                 AVR32_PIN_PA12
#define BOARD_SPI0_MOSI_PIN_FUNCTION        0
#define BOARD_SPI0_CS_PIN                   AVR32_PIN_PA10
#define BOARD_SPI0_CS_PIN_FUNCTION          0

#define BOARD_SPI1_SCK_PIN                  AVR32_PIN_PA15
#define BOARD_SPI1_SCK_PIN_FUNCTION         1
#define BOARD_SPI1_MISO_PIN                 AVR32_PIN_PA17
#define BOARD_SPI1_MISO_PIN_FUNCTION        1
#define BOARD_SPI1_MOSI_PIN                 AVR32_PIN_PA16
#define BOARD_SPI1_MOSI_PIN_FUNCTION        1
#define BOARD_SPI1_CS_PIN                   AVR32_PIN_PA14
#define BOARD_SPI1_CS_PIN_FUNCTION          1

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
  _ROM( AUXLIB_PD, luaopen_pd, pd_map )\
  _ROM( AUXLIB_UART, luaopen_uart, uart_map )\
  _ROM( AUXLIB_PIO, luaopen_pio, pio_map )\
  _ROM( AUXLIB_TMR, luaopen_tmr, tmr_map )\
  _ROM( AUXLIB_TERM, luaopen_term, term_map )\
  _ROM( AUXLIB_CPU, luaopen_cpu, cpu_map )\
  _ROM( AUXLIB_ELUA, luaopen_elua, elua_map )\
  RPCLINE\
  _ROM( AUXLIB_BIT, luaopen_bit, bit_map )\
  _ROM( AUXLIB_PACK, luaopen_pack, pack_map )\
  _ROM( LUA_MATHLIBNAME, luaopen_math, math_map )

/* Minimal ROM modules, to fit in 120KB */
#undef  LUA_PLATFORM_LIBS_ROM
#define LUA_PLATFORM_LIBS_ROM\
  _ROM( AUXLIB_PIO, luaopen_pio, pio_map )\
  _ROM( AUXLIB_TMR, luaopen_tmr, tmr_map )\

// *****************************************************************************
// Configuration data

// Virtual timers (0 if not used)
#define VTMR_NUM_TIMERS       4
#define VTMR_FREQ_HZ          4

// Number of resources (0 if not available/not implemented)
#define NUM_PIO               5
#define NUM_SPI               8
#define NUM_UART              2
#if VTMR_NUM_TIMERS > 0
#define NUM_TIMER             2
#else
#define NUM_TIMER             3
#endif
#define NUM_PWM               0
#define NUM_ADC               0
#define NUM_CAN               0

// RPC boot options
#define RPC_UART_ID           CON_UART_ID
#define RPC_TIMER_ID          CON_TIMER_ID
#define RPC_UART_SPEED        CON_UART_SPEED

// On Mizar32, enabling RX buffering on UART requires the presence of a serial
// board or a 1k resistor between pins 3 and 13 of P5; otherwise the open RX
// input oscillates with the ambient EMI causing many spurious interrupts
// and crashes the interpreter.  Therefore by default we disable the buffering
// so that serial interrupts are not enabled.
//#define BUF_ENABLE_UART
//#define CON_BUF_SIZE          BUF_SIZE_128
// REMEMBER to change next line if buffering is enabled and CON_UART_ID is not 0!
//#define CON_UART_IRQ          AVR32_USART0_IRQ

// SD/MMC Filesystem Setup
#define MMCFS_TICK_HZ                10
#define MMCFS_TICK_MS                ( 1000 / MMCFS_TICK_HZ )
#define MMCFS_SPI_NUM                4
#define MMCFS_CS_PORT                0
#define MMCFS_CS_PIN                 SD_MMC_SPI_NPCS_PIN

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
// On Mizar32, we just use the 32MB SDRAM without trying to use the 8K that is
// free in the onboard 32KB RAM, thereby simplifying the memory management.
//#define MEM_START_ADDRESS     { ( void* )end, ( void* )SDRAM }
//#define MEM_END_ADDRESS       { ( void* )( 0x8000 - STACK_SIZE_TOTAL - 1 ), ( void* )( SDRAM + SDRAM_SIZE - 1 ) }
#define MEM_START_ADDRESS     { ( void* )SDRAM }
#define MEM_END_ADDRESS       { ( void* )( SDRAM + SDRAM_SIZE - 1 ) }

// *****************************************************************************
// CPU constants that should be exposed to the eLua "cpu" module

#endif // #ifndef __MIZAR32_CONF_H__
