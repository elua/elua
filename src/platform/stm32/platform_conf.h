// eLua platform configuration

#ifndef __PLATFORM_CONF_H__
#define __PLATFORM_CONF_H__

#include "auxmods.h"

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
#define EXTENDED_PLATFORM_DATA

// *****************************************************************************
// UART/Timer IDs configuration data (used in main.c)

#define XMODEM_UART_ID        0
#define XMODEM_TIMER_ID       0
#define TERM_UART_ID          0
#define TERM_TIMER_ID         0
#define TERM_LINES            25
#define TERM_COLS             80
#define TERM_TIMEOUT          0

// *****************************************************************************
// Auxiliary libraries that will be compiled for this platform
#if 0
#define LUA_PLATFORM_LIBS\
  { AUXLIB_PIO, luaopen_pio },\
  { AUXLIB_SPI, luaopen_spi },\
  { AUXLIB_TMR, luaopen_tmr },\
  { AUXLIB_PD, luaopen_pd },\
  { AUXLIB_UART, luaopen_uart },\
  { AUXLIB_TERM, luaopen_term },\
  { AUXLIB_PWM, luaopen_pwm },\
  { AUXLIB_PACK, luaopen_pack },\
  { AUXLIB_BIT, luaopen_bit },\
  { AUXLIB_NET, luaopen_net },\
  { AUXLIB_CPU, luaopen_cpu },\
  { LUA_MATHLIBNAME, luaopen_math }
#else
#define LUA_PLATFORM_LIBS\
  { AUXLIB_PIO, luaopen_pio },\
  { AUXLIB_PD, luaopen_pd },\
  { AUXLIB_UART, luaopen_uart },\
  { AUXLIB_TERM, luaopen_term },\
  { AUXLIB_PACK, luaopen_pack },\
  { AUXLIB_BIT, luaopen_bit },\
  { AUXLIB_CPU, luaopen_cpu },\
  { LUA_MATHLIBNAME, luaopen_math }
#endif

// *****************************************************************************
// Configuration data

// Static TCP/IP configuration
#define ELUA_CONF_IPADDR0         192
#define ELUA_CONF_IPADDR1         168
#define ELUA_CONF_IPADDR2         1
#define ELUA_CONF_IPADDR3         13

#define ELUA_CONF_NETMASK0        255
#define ELUA_CONF_NETMASK1        255
#define ELUA_CONF_NETMASK2        255
#define ELUA_CONF_NETMASK3        0

#define ELUA_CONF_DEFGW0          192
#define ELUA_CONF_DEFGW1          168
#define ELUA_CONF_DEFGW2          1
#define ELUA_CONF_DEFGW3          1

#define ELUA_CONF_DNS0            192
#define ELUA_CONF_DNS1            168
#define ELUA_CONF_DNS2            1
#define ELUA_CONF_DNS3            1

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
