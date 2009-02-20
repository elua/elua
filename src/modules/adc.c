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
  int res;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( adc, id );
  res = platform_adc_sample( id );
  
  if ( res != PLATFORM_OK )
    return luaL_error( L, "burst failed" );
  
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

// Lua: samplesready( id )
static int adc_samples_ready( lua_State* L )
{
  unsigned id;
    
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( adc, id );
  lua_pushinteger( L, adc_samples_available( id ) );
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
    if ( res == PLATFORM_ERR )
      return luaL_error( L, "Buffer allocation failed." );
    else
      return 0;
  }
  else
    return luaL_error( L, "length must be power of 2" );

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
  unsigned id, timer_id, count;
  u32 frequency;
  int res;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( adc, id );
  count = luaL_checkinteger( L, 2 );
  timer_id = luaL_checkinteger( L, 3 );
  MOD_CHECK_ID( timer, timer_id );
  frequency = luaL_checkinteger( L, 4 );
  
  if  ( ( count == 0 ) || count & ( count - 1 ) )
    return luaL_error( L, "count must be power of 2 and > 0" );
  
  res = platform_adc_burst( id, intlog2( count ), timer_id, frequency );
  if ( res != PLATFORM_OK )
    return luaL_error( L, "burst failed" );
    
  return 0;
}


// Lua: val = getsample( id )
static int adc_get_sample( lua_State* L )
{
  unsigned id;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( adc, id );
  
  // Wait for pending sampling events to finish (if blocking)
  adc_wait_pending( id );
  
  // If we have at least one sample, return it
  if ( adc_samples_available( id ) >= 1 )
  {
    lua_pushinteger( L, adc_get_processed_sample( id ) );
    return 1;
  }
  return 0;
}


// Lua: table_of_vals = getsamples( id, [count] )
static int adc_get_samples( lua_State* L )
{
  unsigned id, i;
  u16 bcnt, count = 0;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( adc, id );

  if ( lua_isnumber(L, 2) == 1 )
    count = ( u16 )lua_tointeger(L, 2);

  // Wait for any pending operations to finish (if blocking)
  adc_wait_pending( id );
  
  bcnt = adc_samples_available( id );
  
  // If count is zero, grab all samples
  if ( count == 0 )
    count = bcnt;
  
  // Don't pull more samples than are available
  if ( count > bcnt )
    count = bcnt;
  
  lua_createtable( L, count, 0 );
  for( i = 1; i <= count; i ++ )
  {
    lua_pushinteger( L, adc_get_processed_sample( id ) );
    lua_rawseti( L, -2, i );
  }
  return 1;
}


// Module function map
#define MIN_OPT_LEVEL 2
#include "lrodefs.h"
const LUA_REG_TYPE adc_map[] = 
{
  { LSTRKEY( "sample" ), LFUNCVAL( adc_sample ) },
  { LSTRKEY( "maxval" ), LFUNCVAL( adc_maxval ) },
  { LSTRKEY( "samplesready" ), LFUNCVAL( adc_samples_ready ) },
  { LSTRKEY( "setmode" ), LFUNCVAL( adc_set_mode ) },
  { LSTRKEY( "setsmoothing" ), LFUNCVAL( adc_set_smoothing ) },
  { LSTRKEY( "getsmoothing" ), LFUNCVAL( adc_get_smoothing ) },
  { LSTRKEY( "burst" ), LFUNCVAL( adc_burst ) },
  { LSTRKEY( "flush" ), LFUNCVAL( adc_flush ) },
  { LSTRKEY( "getsample" ), LFUNCVAL( adc_get_sample ) },
  { LSTRKEY( "getsamples" ), LFUNCVAL( adc_get_samples ) },
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_adc( lua_State *L )
{
  LREGISTER( L, AUXLIB_ADC, adc_map );
}

#endif
