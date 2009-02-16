// Module for interfacing with ADC

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "auxmods.h"
#include "lrotable.h"
#include "platform_conf.h"
#include "elua_adc.h"

#ifdef BUILD_ADC

// Lua: sample( id )
static int adc_sample( lua_State* L )
{
  unsigned id;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( adc, id );
  platform_adc_sample( id );
  if ( adc_samples_ready( id ) >= 1 )
  {
    lua_pushinteger( L, adc_get_processed_sample( id ) );
    return 1;
  }
  return 0;
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
static int adc_data_ready( lua_State* L )
{
  unsigned id;
  u8 asamp, rsamp;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( adc, id );
  asamp = adc_samples_ready( id );
  rsamp = adc_samples_requested( id );
  if ( rsamp > 0  && asamp >= rsamp )
    lua_pushinteger( L, 1 );
  else
    lua_pushinteger( L, 0 );

  return 1;
}

// Lua: setmode( id, mode )
static int adc_set_mode( lua_State* L )
{
  unsigned id, mode;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( adc, id );
  mode = luaL_checkinteger( L, 2 );
  platform_adc_op( id, PLATFORM_ADC_SET_NONBLOCKING, mode );
  return 0;
}

// Lua: setsmoothing( id, length )
static int adc_set_smoothing( lua_State* L )
{
  unsigned id, length, res;

  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( adc, id );
  length = luaL_checkinteger( L, 2 );
  if ( !( length & ( length - 1 ) ) )
  {
    res = platform_adc_op( id, PLATFORM_ADC_SET_SMOOTHING, length );
    if ( res )
      return luaL_error( L, "Buffer allocation failed." );
    else
      return 0;
  }
  else
    return luaL_error( L, "Smoothing length must be power of 2" );

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

// Lua: flush( id )
static int adc_flush( lua_State* L )
{
  unsigned id;
  u32 res;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( adc, id );
  res = platform_adc_op( id, PLATFORM_ADC_FLUSH, 0 );
  lua_pushinteger( L, res );
  return 1;
}

// Lua: burst( id, count, timer_id, frequency )
static int adc_burst( lua_State* L )
{
  unsigned id, timer_id, i;
  u8 count;
  u32 frequency;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( adc, id );
  count = luaL_checkinteger( L, 2 );
  timer_id = luaL_checkinteger( L, 3 );
  MOD_CHECK_ID( timer, timer_id );
  frequency = luaL_checkinteger( L, 4 );

  if ( count == 0 )
    return 0;
  
  if ( count & ( count - 1 ) )
    return luaL_error( L, "count must be power of 2" );
  
  // If we already have enough data, return it and start next burst
  if( adc_samples_ready( id ) >= count )
  {
    // Push data back to Lua
    lua_createtable( L, count, 0 );
    for( i = 0; i < count; i ++ )
    {
      lua_pushinteger( L, adc_get_processed_sample( id ) );
      lua_rawseti( L, -2, i+1 );
    }
    
    platform_adc_burst( id, count, timer_id, frequency );
    
    return 1;
  }
  else // If no data is available, kick off burst, return data if we have some afterwards
  {
    platform_adc_burst( id, count, timer_id, frequency );
      
    if( adc_samples_ready( id ) >= count )
    {
      // Push data back to Lua
      lua_createtable( L, count, 0 );
      for( i = 0; i < count; i ++ )
      {
        lua_pushinteger( L, adc_get_processed_sample( id ) );
        lua_rawseti( L, -2, i+1 );
      }
      return 1;
    }
  }
  return 0;
}

// Module function map
#define MIN_OPT_LEVEL 2
#include "lrodefs.h"
const LUA_REG_TYPE adc_map[] = 
{
  { LSTRKEY( "sample" ), LFUNCVAL( adc_sample ) },
  { LSTRKEY( "maxval" ), LFUNCVAL( adc_maxval ) },
  { LSTRKEY( "dataready" ), LFUNCVAL( adc_data_ready ) },
  { LSTRKEY( "setmode" ), LFUNCVAL( adc_set_mode ) },
  { LSTRKEY( "setsmoothing" ), LFUNCVAL( adc_set_smoothing ) },
  { LSTRKEY( "getsmoothing" ), LFUNCVAL( adc_get_smoothing ) },
  { LSTRKEY( "burst" ), LFUNCVAL( adc_burst ) },
  { LSTRKEY( "flush" ), LFUNCVAL( adc_flush ) },
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_adc( lua_State *L )
{
  LREGISTER( L, AUXLIB_ADC, adc_map );
}

#endif