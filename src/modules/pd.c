// Module for interfacing with platform data

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "auxmods.h"

static int pd_name( lua_State* L )
{
  lua_pushstring( L, platform_pd_get_name() );
  return 1;
}

// Lua: speed = frequency()
static int pd_clock( lua_State* L )
{
  lua_pushinteger( L, platform_pd_get_cpu_frequency() );
  return 1;
}

// Module function map
static const luaL_reg pd_map[] = 
{
  { "name",  pd_name },
  { "clock", pd_clock },
  { NULL, NULL }
};

LUALIB_API int luaopen_pd( lua_State* L )
{
  luaL_register( L, AUXLIB_PD, pd_map );
  return 1;
}
