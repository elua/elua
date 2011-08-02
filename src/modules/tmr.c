// Module for interfacing with timers

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "auxmods.h"
#include "platform_conf.h"
#include "common.h"
#include "lrotable.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_VTIMER_NAME_LEN     6
#define MIN_VTIMER_NAME_LEN     5

// Helper function for the read/start functions
static int tmrh_timer_op( lua_State* L, int op )
{
  unsigned id;
  timer_data_type res;
    
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( timer, id );
  res = platform_timer_op( id, op, 0 );
  lua_pushinteger( L, res );
  return 1;  
}

// Lua: delay( id, period )
static int tmr_delay( lua_State* L )
{
  unsigned id, period;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( timer, id );
  period = luaL_checkinteger( L, 2 );
  platform_timer_delay( id, period );
  return 0;
}

// Lua: timervalue = read( id )
static int tmr_read( lua_State* L )
{
  return tmrh_timer_op( L, PLATFORM_TIMER_OP_READ );
}

// Lua: timervalue = start( id )
static int tmr_start( lua_State* L )
{
  return tmrh_timer_op( L, PLATFORM_TIMER_OP_START );
}

// Lua: time_us = gettimediff( id, end, start )
static int tmr_gettimediff( lua_State* L )
{
  timer_data_type end, start;
  u32 res;
  unsigned id;
    
  id = luaL_checkinteger( L, 1 ); 
  MOD_CHECK_ID( timer, id );
  end = ( timer_data_type )luaL_checkinteger( L, 2 );
  start = ( timer_data_type )luaL_checkinteger( L, 3 );  
  res = platform_timer_get_diff_us( id, end, start );
  lua_pushinteger( L, res );
  return 1;    
}

// Lua: res = getmindelay( id )
static int tmr_getmindelay( lua_State* L )
{
  u32 res;
  unsigned id;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( timer, id );
  res = platform_timer_op( id, PLATFORM_TIMER_OP_GET_MIN_DELAY, 0 );
  lua_pushinteger( L, res );
  return 1;
}

// Lua: res = getmaxdelay( id )
static int tmr_getmaxdelay( lua_State* L )
{
  u32 res;
  unsigned id;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( timer, id );
  res = platform_timer_op( id, PLATFORM_TIMER_OP_GET_MAX_DELAY, 0 );
  lua_pushinteger( L, res );
  return 1;
}

// Lua: realclock = setclock( id, clock )
static int tmr_setclock( lua_State* L )
{
  u32 clock;
  unsigned id;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( timer, id );
  clock = ( u32 )luaL_checkinteger( L, 2 );
  clock = platform_timer_op( id, PLATFORM_TIMER_OP_SET_CLOCK, clock );
  lua_pushinteger( L, clock );
  return 1;
}

// Lua: clock = getclock( id )
static int tmr_getclock( lua_State* L )
{
  u32 res;
  unsigned id;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( timer, id );
  res = platform_timer_op( id, PLATFORM_TIMER_OP_GET_CLOCK, 0 );
  lua_pushinteger( L, res );
  return 1;
}

#ifdef BUILD_LUA_INT_HANDLERS
// Lua: set_match_int( id, timeout, type )
static int tmr_set_match_int( lua_State *L )
{
  unsigned id;
  u32 res;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( timer, id );
  res = platform_timer_set_match_int( id, ( u32 )luaL_checknumber( L, 2 ), ( int )luaL_checkinteger( L, 3 ) );
  if( res == PLATFORM_TIMER_INT_TOO_SHORT )
    return luaL_error( L, "timer interval too small" );
  else if( res == PLATFORM_TIMER_INT_TOO_LONG )
    return luaL_error( L, "timer interval too long" );
  else if( res == PLATFORM_TIMER_INT_INVALID_ID )
    return luaL_error( L, "match interrupt cannot be set on this timer" );
  return 0;
}
#endif // #ifdef BUILD_LUA_INT_HANDLERS

#if VTMR_NUM_TIMERS > 0
// __index metafunction for TMR
// Look for all VIRTx timer identifiers
static int tmr_mt_index( lua_State* L )
{
  const char *key = luaL_checkstring( L ,2 );
  char* pend;
  long res;
  
  if( strlen( key ) > MAX_VTIMER_NAME_LEN || strlen( key ) < MIN_VTIMER_NAME_LEN )
    return 0;
  if( strncmp( key, "VIRT", 4 ) )
    return 0;  
  res = strtol( key + 4, &pend, 10 );
  if( *pend != '\0' )
    return 0;
  if( res >= VTMR_NUM_TIMERS )
    return 0;
  lua_pushinteger( L, VTMR_FIRST_ID + res );
  return 1;
}
#endif // #if VTMR_NUM_TIMERS > 0

// Module function map
#define MIN_OPT_LEVEL 2
#include "lrodefs.h"
const LUA_REG_TYPE tmr_map[] = 
{
  { LSTRKEY( "delay" ), LFUNCVAL( tmr_delay ) },
  { LSTRKEY( "read" ), LFUNCVAL( tmr_read ) },
  { LSTRKEY( "start" ), LFUNCVAL( tmr_start ) },
  { LSTRKEY( "gettimediff" ), LFUNCVAL( tmr_gettimediff ) },  
  { LSTRKEY( "getmindelay" ), LFUNCVAL( tmr_getmindelay ) },
  { LSTRKEY( "getmaxdelay" ), LFUNCVAL( tmr_getmaxdelay ) },
  { LSTRKEY( "setclock" ), LFUNCVAL( tmr_setclock ) },
  { LSTRKEY( "getclock" ), LFUNCVAL( tmr_getclock ) },
#ifdef BUILD_LUA_INT_HANDLERS
  { LSTRKEY( "set_match_int" ), LFUNCVAL( tmr_set_match_int ) },
#endif  
#if LUA_OPTIMIZE_MEMORY > 0 && VTMR_NUM_TIMERS > 0
  { LSTRKEY( "__metatable" ), LROVAL( tmr_map ) },
#endif
#if VTMR_NUM_TIMERS > 0  
  { LSTRKEY( "__index" ), LFUNCVAL( tmr_mt_index ) },
#endif  
#if LUA_OPTIMIZE_MEMORY > 0 && defined( BUILD_LUA_INT_HANDLERS )
  { LSTRKEY( "INT_ONESHOT" ), LNUMVAL( PLATFORM_TIMER_INT_ONESHOT ) },
  { LSTRKEY( "INT_CYCLIC" ), LNUMVAL( PLATFORM_TIMER_INT_CYCLIC ) },
#endif
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_tmr( lua_State *L )
{
#if LUA_OPTIMIZE_MEMORY > 0
  return 0;
#else // #if LUA_OPTIMIZE_MEMORY > 0
  luaL_register( L, AUXLIB_TMR, tmr_map );
#if VTMR_NUM_TIMERS > 0
  // Set this table as its own metatable
  lua_pushvalue( L, -1 );
  lua_setmetatable( L, -2 );  
#endif // #if VTMR_NUM_TIMERS > 0
  return 1;
#endif // #if LUA_OPTIMIZE_MEMORY > 0
}
