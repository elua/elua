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

// Lua: data = maxval( id )
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

// Lua: realclock = setclock( id, clock, [timer_id] )
static int adc_setclock( lua_State* L )
{
  u32 clock;
  unsigned id, timer_id = 0;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( adc, id );
  clock = luaL_checkinteger( L, 2 );
  if ( clock > 0 )
  {
    timer_id = luaL_checkinteger( L, 3 );
    MOD_CHECK_ID( timer, timer_id );
  }

  platform_adc_op( id, PLATFORM_ADC_OP_SET_TIMER, timer_id );
  clock = platform_adc_op( id, PLATFORM_ADC_OP_SET_CLOCK, clock );
  lua_pushinteger( L, clock );
  return 1;
}

// Lua: data = samplesready( id )
static int adc_samplesready( lua_State* L )
{
  unsigned id;
    
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( adc, id );
  lua_pushinteger( L, adc_samples_available( id ) );
  return 1;
}

// Lua: setblocking( id, mode )
static int adc_setblocking( lua_State* L )
{
  unsigned id, mode;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( adc, id );
  mode = luaL_checkinteger( L, 2 );
  platform_adc_op( id, PLATFORM_ADC_SET_BLOCKING, mode );
  return 0;
}

// Lua: setfreerunning( id, mode )
static int adc_setfreerunning( lua_State* L )
{
  unsigned id, mode;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( adc, id );
  mode = luaL_checkinteger( L, 2 );
  platform_adc_op( id, PLATFORM_ADC_SET_FREERUNNING, mode );
  return 0;
}

// Lua: setsmoothing( id, length )
static int adc_setsmoothing( lua_State* L )
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
static int adc_getsmoothing( lua_State* L )
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

// Lua: sample( id, count )
static int adc_sample( lua_State* L )
{
  unsigned id, count;
  int res;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( adc, id );
  count = luaL_checkinteger( L, 2 );
  
  if  ( ( count == 0 ) || count & ( count - 1 ) )
    return luaL_error( L, "count must be power of 2 and > 0" );
  
  res = platform_adc_sample( id, intlog2( count ) );
  if ( res != PLATFORM_OK )
    return luaL_error( L, "sampling failed" );
    
  return 0;
}


// Lua: val = getsample( id )
static int adc_getsample( lua_State* L )
{
  unsigned id;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( adc, id );
  
  // Wait for pending sampling events to finish (if blocking)
  adc_wait_samples( id, 1 );
  
  // If we have at least one sample, return it
  if ( adc_samples_available( id ) >= 1 )
  {
    lua_pushinteger( L, adc_get_processed_sample( id ) );
    return 1;
  }
  return 0;
}


// Lua: table_of_vals = getsamples( id, [count] )
static int adc_getsamples( lua_State* L )
{
  unsigned id, i;
  u16 bcnt, count = 0;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( adc, id );

  if ( lua_isnumber(L, 2) == 1 )
    count = ( u16 )lua_tointeger(L, 2);

  // Wait for any pending operations to finish (if blocking)
  adc_wait_samples( id, count );
  
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
  { LSTRKEY( "setclock" ), LFUNCVAL( adc_setclock ) },
  { LSTRKEY( "samplesready" ), LFUNCVAL( adc_samplesready ) },
  { LSTRKEY( "setblocking" ), LFUNCVAL( adc_setblocking ) },
  { LSTRKEY( "setsmoothing" ), LFUNCVAL( adc_setsmoothing ) },
  { LSTRKEY( "setfreerunning" ), LFUNCVAL( adc_setfreerunning ) },
  { LSTRKEY( "getsmoothing" ), LFUNCVAL( adc_getsmoothing ) },
  { LSTRKEY( "flush" ), LFUNCVAL( adc_flush ) },
  { LSTRKEY( "getsample" ), LFUNCVAL( adc_getsample ) },
  { LSTRKEY( "getsamples" ), LFUNCVAL( adc_getsamples ) },
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_adc( lua_State *L )
{
  LREGISTER( L, AUXLIB_ADC, adc_map );
}

#endif
