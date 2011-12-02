// eLua platform configuration

// Simplemachines.it Mizar32 board has 128Kbytes of flash with 8kb of bootloader
// To fit in 120K, build using:
// scons board=mizar32 target=lualong optram=0 allocator=newlib

#ifndef __MIZAR32_CONF_H__
#define __MIZAR32_CONF_H__

#include "sdramc.h"
#include "sermux.h"
#include "buf.h"

// *****************************************************************************
// Define here what components you want for this platform

#define BUILD_MMCFS
//#define BUILD_XMODEM
//#define BUILD_SHELL
//#define BUILD_ROMFS
#define BUILD_CON_GENERIC
//#define BUILD_RPC
#define BUF_ENABLE_UART
#define BUILD_C_INT_HANDLERS
#define BUILD_LUA_INT_HANDLERS
//#define BUILD_RFS
//#define BUILD_SERMUX

#if defined( ELUA_CPU_AT32UC3A0128 )
  // Build options for 120KB image
# define RAM_SIZE 0x8000
#else
  // Build options for 256KB and 512KB flash
# define RAM_SIZE 0x10000
# define BUILD_ADC
# define BUILD_LCD
# define BUILD_TERM
# define BUILD_UIP
#endif

#ifdef BUILD_UIP
//#define BUILD_DHCPC
#define BUILD_DNS
//#define BUILD_CON_TCP
#endif

// ****************************************************************************
// Auxiliary libraries that will be compiled for this platform

// The name of the platform specific libs table
#ifdef BUILD_LCD
#define PS_LIB_TABLE_NAME   "mizar32"
#endif

// *****************************************************************************
// UART/Timer IDs configuration data (used in main.c)

#ifdef BUILD_SERMUX
# define CON_UART_ID         ( SERMUX_SERVICE_ID_FIRST + 1 )
#else
# define CON_UART_ID         0
#endif
#define CON_UART_SPEED      115200
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

#ifdef BUILD_ADC
#define ADCLINE _ROM( AUXLIB_ADC, luaopen_adc, adc_map )
#else
#define ADCLINE
#endif

#ifdef BUILD_UIP
#define NETLINE  _ROM( AUXLIB_NET, luaopen_net, net_map )
#else
#define NETLINE
#endif

#if defined( BUILD_RPC )
#define RPCLINE _ROM( AUXLIB_RPC, luaopen_rpc, rpc_map )
#else
#define RPCLINE
#endif

#if defined( PS_LIB_TABLE_NAME )
#define PLATLINE _ROM( PS_LIB_TABLE_NAME, luaopen_platform, platform_map )
#else
#define PLATLINE
#endif

#if defined( ELUA_CPU_AT32UC3A0128 )

// Minimal ROM modules, to fit in 120KB
#define LUA_PLATFORM_LIBS_ROM\
  _ROM( AUXLIB_PD, luaopen_pd, pd_map )\
  _ROM( AUXLIB_PIO, luaopen_pio, pio_map )\
  _ROM( AUXLIB_TMR, luaopen_tmr, tmr_map )\

#else

#define LUA_PLATFORM_LIBS_ROM\
  _ROM( AUXLIB_PD, luaopen_pd, pd_map )\
  _ROM( AUXLIB_UART, luaopen_uart, uart_map )\
  _ROM( AUXLIB_PIO, luaopen_pio, pio_map )\
  _ROM( AUXLIB_PWM, luaopen_pwm, pwm_map )\
  _ROM( AUXLIB_I2C, luaopen_i2c, i2c_map )\
  _ROM( AUXLIB_SPI, luaopen_spi, spi_map )\
  _ROM( AUXLIB_TMR, luaopen_tmr, tmr_map )\
  NETLINE\
  _ROM( AUXLIB_CPU, luaopen_cpu, cpu_map )\
  _ROM( AUXLIB_ELUA, luaopen_elua, elua_map )\
  ADCLINE\
  RPCLINE\
  _ROM( AUXLIB_BIT, luaopen_bit, bit_map )\
  _ROM( AUXLIB_PACK, luaopen_pack, pack_map )\
  _ROM( AUXLIB_TERM, luaopen_term, term_map )\
  _ROM( LUA_MATHLIBNAME, luaopen_math, math_map )\
  PLATLINE\

#endif

// *****************************************************************************
// Configuration data

// Virtual timers (0 if not used)
#define VTMR_NUM_TIMERS       4
#define VTMR_FREQ_HZ          10

// Number of resources (0 if not available/not implemented)
#define NUM_PIO               4
#define NUM_SPI               8
#define NUM_UART              2
#if VTMR_NUM_TIMERS > 0
#define NUM_TIMER             2
#else
#define NUM_TIMER             3
#endif
#define NUM_PWM               6         // PWM7 is on GPIO50
#define NUM_I2C               1
#define NUM_ADC               8         // Though ADC3 pin is the Ethernet IRQ
#define NUM_CAN               0

// As flow control seems not to work, we use a large buffer so that people
// can copy/paste program fragments or data into the serial console.
// An 80x25 screenful is 2000 characters so we use 2048 and the buffer is
// allocated from the 32MB SDRAM so there is no effective limit.
#define CON_BUF_SIZE          BUF_SIZE_2048

// RPC boot options
#define RPC_UART_ID           CON_UART_ID
#define RPC_UART_SPEED        CON_UART_SPEED

// ADC Configuration Params
#define ADC_BIT_RESOLUTION    10
#define BUF_ENABLE_ADC
#define ADC_BUF_SIZE          BUF_SIZE_2

// These should be adjusted to support multiple ADC devices
#define ADC_TIMER_FIRST_ID    0
#define ADC_NUM_TIMERS        0

// SD/MMC Filesystem Setup
#define MMCFS_SPI_NUM          4
#define MMCFS_CS_PORT          0
#define MMCFS_CS_PIN           SD_MMC_SPI_NPCS_PIN

// CPU frequency (needed by the CPU module and MMCFS code, 0 if not used)
#define CPU_FREQUENCY         REQ_CPU_FREQ

// PIO prefix ('0' for P0, P1, ... or 'A' for PA, PB, ...)
#define PIO_PREFIX            'A'
// Pins per port configuration:
// #define PIO_PINS_PER_PORT (n) if each port has the same number of pins, or
// #define PIO_PIN_ARRAY { n1, n2, ... } to define pins per port in an array
// Use #define PIO_PINS_PER_PORT 0 if this isn't needed
#define PIO_PIN_ARRAY         { 31, 32, 32, 14 }
#define AVR32_NUM_GPIO        110 // actually 109, but consider also PA31

#ifdef BOOTLOADER_EMBLOD
# define ELUA_FIRMWARE_SIZE 0x80000
#else
# define ELUA_FIRMWARE_SIZE 0
#endif

// Allocator data: define your free memory zones here in two arrays
// (start address and end address)
#define MEM_START_ADDRESS     { ( void* )end, ( void* )( SDRAM + ELUA_FIRMWARE_SIZE ) }
#define MEM_END_ADDRESS       { ( void* )( RAM_SIZE - STACK_SIZE_TOTAL - 1 ), ( void* )( SDRAM + SDRAM_SIZE - 1 ) }

#define RFS_BUFFER_SIZE       BUF_SIZE_512
#define RFS_UART_ID           ( SERMUX_SERVICE_ID_FIRST )
#define RFS_TIMEOUT           100000
#define RFS_UART_SPEED        115200

//#define SERMUX_PHYS_ID        0
//#define SERMUX_PHYS_SPEED     115200
//#define SERMUX_NUM_VUART      2
//#define SERMUX_BUFFER_SIZES   { RFS_BUFFER_SIZE, CON_BUF_SIZE }

// *****************************************************************************
// CPU constants that should be exposed to the eLua "cpu" module


// Static TCP/IP configuration

#define ELUA_CONF_IPADDR0     192
#define ELUA_CONF_IPADDR1     168
#define ELUA_CONF_IPADDR2     1
#define ELUA_CONF_IPADDR3     10

#define ELUA_CONF_NETMASK0    255
#define ELUA_CONF_NETMASK1    255
#define ELUA_CONF_NETMASK2    255
#define ELUA_CONF_NETMASK3    0

#define ELUA_CONF_DEFGW0      192
#define ELUA_CONF_DEFGW1      168
#define ELUA_CONF_DEFGW2      1
#define ELUA_CONF_DEFGW3      1

#define ELUA_CONF_DNS0        192
#define ELUA_CONF_DNS1        168
#define ELUA_CONF_DNS2        1
#define ELUA_CONF_DNS3        1

#endif // #ifndef __MIZAR32_CONF_H__

