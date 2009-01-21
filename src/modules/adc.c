// Module for interfacing with ADC

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "auxmods.h"
#include "lrotable.h"
#include "platform_conf.h"

// Lua: sample( id )
static int adc_sample( lua_State* L )
{
  unsigned id;
  u16 res;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( adc, id );
  res = platform_adc_sample( id );
  lua_pushinteger( L, res );
  return 1;
}

// Lua: start( id )
static int adc_start( lua_State* L )
{
  unsigned id;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( adc, id );
  platform_adc_start( id );
  return 0;
}

// Lua: maxval( id )
static int adc_maxval( lua_State* L)
{
  unsigned id;
  u16 res;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( adc, id );
  res = platform_adc_maxval( id );
  lua_pushinteger( L, res );
  return 1;
}

// Lua: is_done( id )
static int adc_is_done( lua_State* L )
{
  unsigned id;
  int res;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( adc, id );
  res = platform_adc_is_done( id );
  lua_pushinteger( L, res );
  return 1;
}

// Lua: set_mode( id, mode )
static int adc_set_mode( lua_State* L )
{
  unsigned id, mode;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( adc, id );
  mode = luaL_checkinteger( L, 2 );
  platform_adc_set_mode( id, mode );
  return 0;
}

// Module function map
#define MIN_OPT_LEVEL 2
#include "lrodefs.h"
const LUA_REG_TYPE adc_map[] = 
{
  { LSTRKEY( "sample" ), LFUNCVAL( adc_sample ) },
  { LSTRKEY( "start" ), LFUNCVAL( adc_start ) },
  { LSTRKEY( "maxval" ), LFUNCVAL( adc_maxval ) },
  { LSTRKEY( "isdone" ), LFUNCVAL( adc_is_done ) },
  { LSTRKEY( "setmode" ), LFUNCVAL( adc_set_mode ) },
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_adc( lua_State *L )
{
  LREGISTER( L, AUXLIB_ADC, adc_map );
}