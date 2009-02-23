// eLua platform configuration

#ifndef __PLATFORM_CONF_H__
#define __PLATFORM_CONF_H__

#include "auxmods.h"
#include "type.h"
#include "stacks.h"

// *****************************************************************************
// Define here what components you want for this platform

//#define BUILD_XMODEM
#define BUILD_SHELL
#define BUILD_ROMFS
#define BUILD_TERM
//#define BUILD_UIP
//#define BUILD_DHCPC
//#define BUILD_DNS
#define BUILD_CON_GENERIC
//#define BUILD_CON_TCP

// *****************************************************************************
// UART/Timer IDs configuration data (used in main.c)

#define CON_UART_ID           0
#define CON_UART_SPEED        115200
#define XMODEM_TIMER_ID       0
#define TERM_TIMER_ID         0
#define TERM_LINES            25
#define TERM_COLS             80
#define TERM_TIMEOUT          100000

// *****************************************************************************
// Auxiliary libraries that will be compiled for this platform

#ifdef FORSTM3210E_EVAL
#define AUXLIB_LCD      "stm3210lcd"
LUALIB_API int ( luaopen_lcd )( lua_State* L );
#define LCDLINE  _ROM( AUXLIB_LCD, luaopen_lcd, lcd_map )
#else
#define LCDLINE
#endif

#define LUA_PLATFORM_LIBS_ROM\
  _ROM( AUXLIB_PIO, luaopen_pio, pio_map )\
  _ROM( AUXLIB_PD, luaopen_pd, pd_map )\
  _ROM( AUXLIB_UART, luaopen_uart, uart_map )\
  _ROM( AUXLIB_TERM, luaopen_term, term_map )\
  _ROM( AUXLIB_PACK, luaopen_pack, pack_map )\
  _ROM( AUXLIB_BIT, luaopen_bit, bit_map )\
  _ROM( AUXLIB_CPU, luaopen_cpu, cpu_map )\
  LCDLINE\
  _ROM( LUA_MATHLIBNAME, luaopen_math, math_map )

// *****************************************************************************
// Configuration data

// Static TCP/IP configuration
#define ELUA_CONF_IPADDR0     192
#define ELUA_CONF_IPADDR1     168
#define ELUA_CONF_IPADDR2     1
#define ELUA_CONF_IPADDR3     13

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

// *****************************************************************************
// Configuration data

// Virtual timers (0 if not used)
#define VTMR_NUM_TIMERS       0
#define VTMR_FREQ_HZ          4

// Number of resources (0 if not available/not implemented)
#define NUM_PIO               7
#define NUM_SPI               0
#define NUM_UART              4
#define NUM_TIMER             0
#define NUM_PWM               0
#define NUM_ADC               0

// CPU frequency (needed by the CPU module, 0 if not used)
u32 platform_s_cpu_get_frequency();
#define CPU_FREQUENCY         platform_s_cpu_get_frequency()

// PIO prefix ('0' for P0, P1, ... or 'A' for PA, PB, ...)
#define PIO_PREFIX            'A'
// Pins per port configuration:
// #define PIO_PINS_PER_PORT (n) if each port has the same number of pins, or
// #define PIO_PIN_ARRAY { n1, n2, ... } to define pins per port in an array
// Use #define PIO_PINS_PER_PORT 0 if this isn't needed
#define PIO_PIN_ARRAY         { 16, 16, 16, 16, 16, 16, 16 }

// Allocator data: define your free memory zones here in two arrays
// (start address and end address)
#define SRAM_SIZE             ( 64 * 1024 )
#define MEM_START_ADDRESS     { ( void* )end }
#define MEM_END_ADDRESS       { ( void* )( SRAM_BASE + SRAM_SIZE - STACK_SIZE_TOTAL - 1 ) }

// *****************************************************************************
// CPU constants that should be exposed to the eLua "cpu" module

#include "stm32f10x_gpio.h"

#if 0
#define PLATFORM_CPU_CONSTANTS\
  _C( INT_GPIOA ),\
  _C( INT_GPIOB ),\
  _C( INT_GPIOC ),\
  _C( INT_GPIOD ),\
  _C( INT_GPIOE ),\
  _C( INT_UART0 ),\
  _C( INT_UART1 ),\
  _C( INT_SSI0 ),\
  _C( INT_I2C0 ),\
  _C( INT_PWM_FAULT ),\
  _C( INT_PWM0 ),\
  _C( INT_PWM1 ),\
  _C( INT_PWM2 ),\
  _C( INT_QEI0 ),\
  _C( INT_ADC0 ),\
  _C( INT_ADC1 ),\
  _C( INT_ADC2 ),\
  _C( INT_ADC3 ),\
  _C( INT_WATCHDOG ),\
  _C( INT_TIMER0A ),\
  _C( INT_TIMER0B ),\
  _C( INT_TIMER1A ),\
  _C( INT_TIMER1B ),\
  _C( INT_TIMER2A ),\
  _C( INT_TIMER2B ),\
  _C( INT_COMP0 ),\
  _C( INT_COMP1 ),\
  _C( INT_COMP2 ),\
  _C( INT_SYSCTL ),\
  _C( INT_FLASH ),\
  _C( INT_GPIOF ),\
  _C( INT_GPIOG ),\
  _C( INT_GPIOH ),\
  _C( INT_UART2 ),\
  _C( INT_SSI1 ),\
  _C( INT_TIMER3A ),\
  _C( INT_TIMER3B ),\
  _C( INT_I2C1 ),\
  _C( INT_QEI1 ),\
  _C( INT_CAN0 ),\
  _C( INT_CAN1 ),\
  _C( INT_CAN2 ),\
  _C( INT_ETH ),\
  _C( INT_HIBERNATE ),\
  _C( INT_USB0 ),\
  _C( INT_PWM3 ),\
  _C( INT_UDMA ),\
  _C( INT_UDMAERR )
#endif

#endif // #ifndef __PLATFORM_CONF_H__
