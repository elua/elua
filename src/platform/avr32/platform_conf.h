// eLua platform configuration

#ifndef __PLATFORM_CONF_H__
#define __PLATFORM_CONF_H__

#include "auxmods.h"

// *****************************************************************************
// Define here what components you want for this platform

#define BUILD_XMODEM
#define BUILD_SHELL
#define BUILD_ROMFS
#define BUILD_TERM
//#define BUILD_UIP
//#define BUILD_DNS
#define BUILD_CON_GENERIC

// *****************************************************************************
// UART/Timer IDs configuration data (used in main.c)

// [TODO] added CON_UART_xxxx below, add it to all backends, and probably remove
// XMODEM_UART_ID (although this isn't strictly required)
#define CON_UART_ID           0
#define CON_UART_SPEED        38400
#define XMODEM_UART_ID        0
#define XMODEM_TIMER_ID       0
#define TERM_UART_ID          0
#define TERM_TIMER_ID         0
#define TERM_LINES            25
#define TERM_COLS             80
#define TERM_TIMEOUT          100000

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
  { LUA_MATHLIBNAME, luaopen_math }

// *****************************************************************************
// Configuration data

// Static TCP/IP configuration
#define ELUA_CONF_IPADDR0         192
#define ELUA_CONF_IPADDR1         168
#define ELUA_CONF_IPADDR2         1
#define ELUA_CONF_IPADDR3         218

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

#endif // #ifndef __PLATFORM_CONF_H__
