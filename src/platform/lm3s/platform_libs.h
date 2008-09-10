// Auxiliary libraries that will be compiled for this platform

#ifndef __PLATFORM_LIBS_H__
#define __PLATFORM_LIBS_H__

#include "auxmods.h"

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
  { LUA_MATHLIBNAME, luaopen_math }
  
#endif
