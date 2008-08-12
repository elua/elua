// Auxiliary libraries that will be compiled for this platform

#ifndef __PLATFORM_LIBS_H__
#define __PLATFORM_LIBS_H__

#include "auxmods.h"

#define LUA_PLATFORM_LIBS\
  { AUXLIB_PIO, luaopen_pio },\
  { AUXLIB_TMR, luaopen_tmr },\
  { AUXLIB_PD, luaopen_pd },\
  { AUXLIB_UART, luaopen_uart },\
  { AUXLIB_TERM, luaopen_term }
  
#endif
