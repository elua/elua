// Auxiliary Lua modules. All of them are declared here, then each platform
// decides what module(s) to register in the "platform_libs.h" file

#ifndef __AUXMODS_H__
#define __AUXMODS_H__

#include "lua.h"

#define AUXLIB_PIO      "pio"
LUALIB_API int ( luaopen_pio )( lua_State *L );

#define AUXLIB_SPI      "spi"
LUALIB_API int ( luaopen_spi )( lua_State* L );

#define AUXLIB_TMR      "tmr"
LUALIB_API int ( luaopen_tmr )( lua_State* L );

#define AUXLIB_PD       "pd"
LUALIB_API int ( luaopen_pd )( lua_State* L );

#define AUXLIB_UART     "uart"
LUALIB_API int ( luaopen_uart )( lua_State* L );

#define AUXLIB_TERM     "term"
LUALIB_API int ( luaopen_term )( lua_State* L );

#ifdef ELUA_MOD_CHECK_PARS

// Helper macros
#define MOD_CHECK_MIN_ARGS( num )\
  if( lua_gettop( L ) < num )\
  {\
    lua_pushnil( L );\
    return 1;\
  }

#define MOD_CHECK_MIN_ARGS_INT( num, res )\
  if( lua_gettop( L ) < num )\
  {\
    lua_pushinteger( L, res );\
    return 1;\
  }  
  
#else

#define MOD_CHECK_MIN_ARGS( num )
#define MOD_CHECK_MIN_ARGS_INT( num, res )
  
#endif

#endif
