// Module for interfacing with ADC

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "auxmods.h"
#include "lrotable.h"
#include "platform_conf.h"
#include <stdlib.h> // needed for malloc

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

// Lua: maxval( id )
static int adc_maxval( lua_State* L )
{
  unsigned id;
  u32 res;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( adc, id );
  res = platform_adc_op( id, PLATFORM_ADC_GET_MAXVAL, 0 );
  lua_pushinteger( L, res );
  return 1;
}

// Lua: isdone( id )
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

// Lua: setmode( id, mode )
static int adc_set_mode( lua_State* L )
{
  unsigned id, mode;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( adc, id );
  mode = luaL_checkinteger( L, 2 );
  platform_adc_set_mode( id, mode );
  return 0;
}

// Lua: setsmoothing( id, length )
static int adc_set_smoothing( lua_State* L )
{
  unsigned id, length, res;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( adc, id );
  length = luaL_checkinteger( L, 2 );
  res = platform_adc_op( id, PLATFORM_ADC_SET_SMOOTHING, length );
  if ( res )
    return luaL_error( L, "Buffer allocation failed." );
  else
    return 0;
}

// Lua: getsmoothing( id )
static int adc_get_smoothing( lua_State* L )
{
  unsigned id;
  u32 res;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( adc, id );
  res = platform_adc_op( id, PLATFORM_ADC_GET_SMOOTHING, 0 );
  lua_pushinteger( L, res );
  return 1;
}

// Lua: burst( id, count, timer_id, frequency )
static int adc_burst( lua_State* L )
{
  unsigned i, id, count, timer_id;
  u32 frequency;
  u16 *buf;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( adc, id );
  count = luaL_checkinteger( L, 2 );
  timer_id = luaL_checkinteger( L, 3 );
  MOD_CHECK_ID( timer, timer_id );
  frequency = luaL_checkinteger( L, 4 );
  
  // Allocate buffer to contain returned samples
  if( ( buf = malloc( count * sizeof( u16 ) ) ) == NULL )
    return luaL_error( L, "Buffer allocation failed." );
  
  for( i = 0; i < count; i ++ )
    buf[ i ] = 0;
  
  platform_adc_burst( id, buf, count, timer_id, frequency );

  // Push data back to Lua
  lua_createtable( L, count, 0 );
  for( i = 0; i < count; i ++ )
  {
    lua_pushinteger( L, buf[ i ] );
    lua_rawseti( L, -2, i+1 );
  }
  
  // Free buffer
  free( buf );
  
  return 1;
}

// Module function map
#define MIN_OPT_LEVEL 2
#include "lrodefs.h"
const LUA_REG_TYPE adc_map[] = 
{
  { LSTRKEY( "sample" ), LFUNCVAL( adc_sample ) },
  { LSTRKEY( "maxval" ), LFUNCVAL( adc_maxval ) },
  { LSTRKEY( "isdone" ), LFUNCVAL( adc_is_done ) },
  { LSTRKEY( "setmode" ), LFUNCVAL( adc_set_mode ) },
  { LSTRKEY( "setsmoothing" ), LFUNCVAL( adc_set_smoothing ) },
  { LSTRKEY( "getsmoothing" ), LFUNCVAL( adc_get_smoothing ) },
  { LSTRKEY( "burst" ), LFUNCVAL( adc_burst ) },
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_adc( lua_State *L )
{
  LREGISTER( L, AUXLIB_ADC, adc_map );
}
