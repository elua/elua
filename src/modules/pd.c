// Module for interfacing with platform data

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "auxmods.h"

// Lua: platform = platform()
static int pd_platform( lua_State* L )
{
  lua_pushstring( L, platform_pd_get_platform_name() );
  return 1;
}

// Lua: cpuname = cpu()
static int pd_cpu( lua_State* L )
{
  lua_pushstring( L, platform_pd_get_cpu_name() );
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
//FIXME: remove "name" once web site samples use platform()
  { "name",  pd_platform },
  { "platform",  pd_platform }, 
  { "cpu", pd_cpu },
  { "clock", pd_clock },
  { NULL, NULL }
};

LUALIB_API int luaopen_pd( lua_State* L )
{
  luaL_register( L, AUXLIB_PD, pd_map );
  return 1;
}
