// eLua platform configuration

#ifndef __PLATFORM_CONF_H__
#define __PLATFORM_CONF_H__

#include "auxmods.h"

// *****************************************************************************
// Define here what components you want for this platform

#define BUILD_SHELL
#define BUILD_ROMFS
#define BUILD_CON_GENERIC

// *****************************************************************************
// Auxiliary libraries that will be compiled for this platform

#define LUA_PLATFORM_LIBS\
  { AUXLIB_PD, luaopen_pd },\
  { LUA_MATHLIBNAME, luaopen_math }
  
#endif // #ifndef __PLATFORM_CONF_H__
