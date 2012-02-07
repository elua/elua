// Module for interfacing with ADC

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "common.h"
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
  res = platform_adc_get_maxval( id );
  lua_pushinteger( L, res );
  return 1;
}

// Lua: realclock = setclock( id, freq, [timer_id] )
static int adc_setclock( lua_State* L )
{
  s32 sfreq; // signed version for negative checking
  u32 freq;
  unsigned id, timer_id = 0;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( adc, id );
  sfreq = luaL_checkinteger( L, 2 );
  if ( sfreq < 0 )
    return luaL_error( L, "frequency must be 0 or positive" );
  freq = ( u32 ) sfreq;
  if ( freq > 0 )
  {
    timer_id = luaL_checkinteger( L, 3 );
    MOD_CHECK_ID( timer, timer_id );
    MOD_CHECK_RES_ID( adc, id, timer, timer_id );
  }

  platform_adc_set_timer( id, timer_id );
  freq = platform_adc_set_clock( id, freq );
  lua_pushinteger( L, freq );
  return 1;
}

// Lua: data = isdone( id )
static int adc_isdone( lua_State* L )
{
  unsigned id;
    
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( adc, id );
  lua_pushinteger( L, platform_adc_is_done( id ) );
  return 1;
}

// Lua: setblocking( id, mode )
static int adc_setblocking( lua_State* L )
{
  unsigned id, mode;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( adc, id );
  mode = luaL_checkinteger( L, 2 );
  platform_adc_set_blocking( id, mode );
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
    res = platform_adc_set_smoothing( id, length );
    if ( res == PLATFORM_ERR )
      return luaL_error( L, "Buffer allocation failed." );
    else
      return 0;
  }
  else
    return luaL_error( L, "length must be power of 2" );
}

// Lua: sample( id, count )
static int adc_sample( lua_State* L )
{
  unsigned id, count, nchans = 1;
  int res, i;  
  
  count = luaL_checkinteger( L, 2 );
  if  ( ( count == 0 ) || count & ( count - 1 ) )
    return luaL_error( L, "count must be power of 2 and > 0" );
  
  // If first parameter is a table, extract channel list
  if ( lua_istable( L, 1 ) == 1 )
  {
    nchans = lua_objlen(L, 1);
    
    // Get/check list of channels and setup
    for( i = 0; i < nchans; i++ )
    {
      lua_rawgeti( L, 1, i+1 );
      id = luaL_checkinteger( L, -1 );
      MOD_CHECK_ID( adc, id );
      
      res = adc_setup_channel( id, intlog2( count ) );
      if ( res != PLATFORM_OK )
        return luaL_error( L, "sampling setup failed" );
    }
    // Initiate sampling
    platform_adc_start_sequence();
  }
  else if ( lua_isnumber( L, 1 ) == 1 )
  {
    id = luaL_checkinteger( L, 1 );
    MOD_CHECK_ID( adc, id );
    
    res = adc_setup_channel( id, intlog2( count ) );
    if ( res != PLATFORM_OK )
      return luaL_error( L, "sampling setup failed" );
    
    platform_adc_start_sequence();
  }
  else
  {
    return luaL_error( L, "invalid channel selection" );
  }
  return 0;
}


// Lua: val = getsample( id )
static int adc_getsample( lua_State* L )
{
  unsigned id;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( adc, id );
  
  // If we have at least one sample, return it
  if ( adc_wait_samples( id, 1 ) >= 1 )
  {
    lua_pushinteger( L, adc_get_processed_sample( id ) );
    return 1;
  }
  return 0;
}

#if defined( BUF_ENABLE_ADC )
// Lua: table_of_vals = getsamples( id, [count] )
static int adc_getsamples( lua_State* L )
{
  unsigned id, i;
  u16 bcnt, count = 0;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( adc, id );

  if ( lua_isnumber(L, 2) == 1 )
    count = ( u16 )lua_tointeger(L, 2);
  
  bcnt = adc_wait_samples( id, count );
  
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


// Lua: insertsamples(id, table, idx, count)
static int adc_insertsamples( lua_State* L )
{
  unsigned id, i, startidx;
  u16 bcnt, count;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( adc, id );
  
  luaL_checktype(L, 2, LUA_TTABLE);
  
  startidx = luaL_checkinteger( L, 3 );
  if  ( startidx <= 0 )
    return luaL_error( L, "idx must be > 0" );

  count = luaL_checkinteger(L, 4 );
  if  ( count == 0 )
    return luaL_error( L, "count must be > 0" );
  
  bcnt = adc_wait_samples( id, count );
  
  for( i = startidx; i < ( count + startidx ); i ++ )
  {
    if ( i < bcnt + startidx )
      lua_pushinteger( L, adc_get_processed_sample( id ) );
    else
      lua_pushnil( L ); // nil-out values where we don't have enough samples

    lua_rawseti( L, 2, i );
  }
  
  return 0;
}
#endif

// Module function map
#define MIN_OPT_LEVEL 2
#include "lrodefs.h"
const LUA_REG_TYPE adc_map[] = 
{
  { LSTRKEY( "sample" ), LFUNCVAL( adc_sample ) },
  { LSTRKEY( "maxval" ), LFUNCVAL( adc_maxval ) },
  { LSTRKEY( "setclock" ), LFUNCVAL( adc_setclock ) },
  { LSTRKEY( "isdone" ), LFUNCVAL( adc_isdone ) },
  { LSTRKEY( "setblocking" ), LFUNCVAL( adc_setblocking ) },
  { LSTRKEY( "setsmoothing" ), LFUNCVAL( adc_setsmoothing ) },
  { LSTRKEY( "getsample" ), LFUNCVAL( adc_getsample ) },
#if defined( BUF_ENABLE_ADC )
  { LSTRKEY( "getsamples" ), LFUNCVAL( adc_getsamples ) },
  { LSTRKEY( "insertsamples" ), LFUNCVAL( adc_insertsamples ) },
#endif
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_adc( lua_State *L )
{
  LREGISTER( L, AUXLIB_ADC, adc_map );
}

#endif
