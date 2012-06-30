// eLua platform configuration

#ifndef __PLATFORM_CONF_H__
#define __PLATFORM_CONF_H__

#include "auxmods.h"
#include "type.h"
#include "stacks.h"
#include "stm32f10x.h"
#include "elua_int.h"
#include "sermux.h"

// *****************************************************************************
// Define here what components you want for this platform

#define BUILD_XMODEM
#define BUILD_SHELL
#define BUILD_ROMFS
//#define BUILD_MMCFS
#define BUILD_TERM
//#define BUILD_UIP
//#define BUILD_DHCPC
//#define BUILD_DNS
#define BUILD_CON_GENERIC
#define BUILD_ADC
#define BUILD_RPC
//#define BUILD_RFS
//#define BUILD_CON_TCP
#define BUILD_LINENOISE
#define BUILD_C_INT_HANDLERS
#define BUILD_LUA_INT_HANDLERS
#define ENABLE_ENC

#define PLATFORM_HAS_SYSTIMER

// *****************************************************************************
// UART/Timer IDs configuration data (used in main.c)

#define CON_UART_ID           0
#define CON_UART_SPEED        115200
#define TERM_LINES            25
#define TERM_COLS             80

// *****************************************************************************
// Auxiliary libraries that will be compiled for this platform

//#ifdef FORSTM3210E_EVAL
//#define AUXLIB_LCD      "stm3210lcd"
//LUALIB_API int ( luaopen_lcd )( lua_State* L );
//#define LCDLINE  _ROM( AUXLIB_LCD, luaopen_lcd, lcd_map )
//#else
#define LCDLINE
//#endif

#ifdef ENABLE_ENC
#define PS_LIB_TABLE_NAME "stm32"
#endif


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

#ifdef PS_LIB_TABLE_NAME
#define PLATLINE _ROM( PS_LIB_TABLE_NAME, luaopen_platform, platform_map )
#else
#define PLATLINE
#endif

#define LUA_PLATFORM_LIBS_ROM\
  _ROM( AUXLIB_PIO, luaopen_pio, pio_map )\
  _ROM( AUXLIB_SPI, luaopen_spi, spi_map )\
  _ROM( AUXLIB_PD, luaopen_pd, pd_map )\
  _ROM( AUXLIB_UART, luaopen_uart, uart_map )\
  _ROM( AUXLIB_TERM, luaopen_term, term_map )\
  _ROM( AUXLIB_PACK, luaopen_pack, pack_map )\
  _ROM( AUXLIB_BIT, luaopen_bit, bit_map )\
  _ROM( AUXLIB_CPU, luaopen_cpu, cpu_map )\
  _ROM( AUXLIB_ELUA, luaopen_elua, elua_map )\
  _ROM( AUXLIB_TMR, luaopen_tmr, tmr_map )\
  ADCLINE\
  _ROM( AUXLIB_CAN, luaopen_can, can_map )\
  _ROM( AUXLIB_PWM, luaopen_pwm, pwm_map )\
  RPCLINE\
  LCDLINE\
  _ROM( AUXLIB_ELUA, luaopen_elua, elua_map )\
  _ROM( LUA_MATHLIBNAME, luaopen_math, math_map )\
  PLATLINE

// *****************************************************************************
// Configuration data

#define EGC_INITIAL_MODE      1

// Virtual timers (0 if not used)
#define VTMR_NUM_TIMERS       4
#define VTMR_FREQ_HZ          10

// Number of resources (0 if not available/not implemented)
#define NUM_PIO               7
#define NUM_SPI               2
#define NUM_UART              5
#define NUM_TIMER             5
#define NUM_PHYS_TIMER        5
#define NUM_PWM               4
#define NUM_ADC               16
#define NUM_CAN               1

// Enable RX buffering on UART
#define BUF_ENABLE_UART
#define CON_BUF_SIZE          BUF_SIZE_128

// ADC Configuration Params
#define ADC_BIT_RESOLUTION    12
#define BUF_ENABLE_ADC
#define ADC_BUF_SIZE          BUF_SIZE_2

// These should be adjusted to support multiple ADC devices
#define ADC_TIMER_FIRST_ID    0
#define ADC_NUM_TIMERS        4

// RPC boot options
#define RPC_UART_ID           CON_UART_ID
#define RPC_UART_SPEED        CON_UART_SPEED




// MMCFS Support (FatFs on SD/MMC)
// For STM32F103RET6 - PA5 = CLK, PA6 = MISO, PA7 = MOSI, PA8 = CS
#define MMCFS_CS_PORT                0
#define MMCFS_CS_PIN                 8
#define MMCFS_SPI_NUM                0

// CPU frequency (needed by the CPU module, 0 if not used)
u32 platform_s_cpu_get_frequency();
#define CPU_FREQUENCY         platform_s_cpu_get_frequency()

// PIO prefix ('0' for P0, P1, ... or 'A' for PA, PB, ...)
#define PIO_PREFIX            'A'
// Pins per port configuration:
// #define PIO_PINS_PER_PORT (n) if each port has the same number of pins, or
// #define PIO_PIN_ARRAY { n1, n2, ... } to define pins per port in an array
// Use #define PIO_PINS_PER_PORT 0 if this isn't needed
#define PIO_PINS_PER_PORT     16

// Remote file system data
#define RFS_BUFFER_SIZE       BUF_SIZE_512
#define RFS_UART_ID           0
#define RFS_TIMEOUT           100000
#define RFS_UART_SPEED        115200

// Linenoise buffer sizes
#define LINENOISE_HISTORY_SIZE_LUA    50
#define LINENOISE_HISTORY_SIZE_SHELL  10

// Allocator data: define your free memory zones here in two arrays
// (start address and end address)
#define SRAM_SIZE             ( 64 * 1024 )
#define MEM_START_ADDRESS     { ( void* )end }
#define MEM_END_ADDRESS       { ( void* )( SRAM_BASE + SRAM_SIZE - STACK_SIZE_TOTAL - 1 ) }

// Flash data (only for STM32F103RE for now)
#ifdef ELUA_CPU_STM32F103RE
#define INTERNAL_FLASH_SIZE             ( 512 * 1024 )
#define INTERNAL_FLASH_SECTOR_SIZE      2048
#define INTERNAL_FLASH_START_ADDRESS    0x08000000
#define BUILD_WOFS
#endif // #ifdef ELUA_CPU_STM32F103RE

// Interrupt queue size
#define PLATFORM_INT_QUEUE_LOG_SIZE 5

// Interrupt list
#define INT_GPIO_POSEDGE      ELUA_INT_FIRST_ID
#define INT_GPIO_NEGEDGE      ( ELUA_INT_FIRST_ID + 1 )
#define INT_TMR_MATCH         ( ELUA_INT_FIRST_ID + 2 )
#define INT_UART_RX           ( ELUA_INT_FIRST_ID + 3 )
#define INT_ELUA_LAST         INT_UART_RX

#define PLATFORM_CPU_CONSTANTS\
  _C( INT_GPIO_POSEDGE ),     \
  _C( INT_GPIO_NEGEDGE ),     \
  _C( INT_TMR_MATCH ),        \
  _C( INT_UART_RX )

#endif // #ifndef __PLATFORM_CONF_H__

