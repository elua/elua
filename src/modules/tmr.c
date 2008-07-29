// Module for interfacing with timers

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "auxmods.h"

// Helper function for the read/start functions
static int tmrh_timer_op( lua_State* L, int op )
{
  unsigned id;
  timer_data_type res;
    
  MOD_CHECK_MIN_ARGS( 1 );  
  id = luaL_checkinteger( L, 1 );
  res = platform_timer_op( id, op, 0 );
  lua_pushinteger( L, res );
  return 1;  
}

// Lua: delay( id, period )
static int tmr_delay( lua_State* L )
{
  unsigned id, period;
  
  MOD_CHECK_MIN_ARGS( 2 );
  id = luaL_checkinteger( L, 1 );
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

// Lua: time_us = diff( id, end, start )
static int tmr_diff( lua_State* L )
{
  timer_data_type end, start;
  u32 res;
  unsigned id;
    
  MOD_CHECK_MIN_ARGS( 3 );
  id = luaL_checkinteger( L, 1 ); 
  end = luaL_checkinteger( L, 2 );
  start = luaL_checkinteger( L, 3 );  
  res = platform_timer_get_diff_us( id, end, start );
  lua_pushinteger( L, res );
  return 1;    
}

// Lua: res = mindelay( id )
static int tmr_mindelay( lua_State* L )
{
  u32 res;
  unsigned id;
  
  MOD_CHECK_MIN_ARGS( 1 );
  id = luaL_checkinteger( L, 1 );
  res = platform_timer_op( id, PLATFORM_TIMER_OP_GET_MIN_DELAY, 0 );
  lua_pushinteger( L, res );
  return 1;
}

// Lua: res = maxdelay( id )
static int tmr_maxdelay( lua_State* L )
{
  u32 res;
  unsigned id;
  
  MOD_CHECK_MIN_ARGS( 1 );
  id = luaL_checkinteger( L, 1 );
  res = platform_timer_op( id, PLATFORM_TIMER_OP_GET_MAX_DELAY, 0 );
  lua_pushinteger( L, res );
  return 1;
}

// Lua: realclock = setclock( id, clock )
static int tmr_setclock( lua_State* L )
{
  u32 clock;
  unsigned id;
  
  MOD_CHECK_MIN_ARGS( 2 );
  id = luaL_checkinteger( L, 1 );
  clock = luaL_checkinteger( L, 2 );
  clock = platform_timer_op( id, PLATFORM_TIMER_OP_SET_CLOCK, clock );
  lua_pushinteger( L, clock );
  return 1;
}

// Lua: clock = getclock( id )
static int tmr_getclock( lua_State* L )
{
  u32 res;
  unsigned id;
  
  MOD_CHECK_MIN_ARGS( 1 );
  id = luaL_checkinteger( L, 1 );
  res = platform_timer_op( id, PLATFORM_TIMER_OP_GET_CLOCK, 0 );
  lua_pushinteger( L, res );
  return 1;
}

// Module function map
static const luaL_reg tmr_map[] = 
{
  { "delay",  tmr_delay },
  { "read", tmr_read },
  { "start", tmr_start },
  { "diff", tmr_diff },  
  { "mindelay", tmr_mindelay },
  { "maxdelay", tmr_maxdelay },
  { "setclock", tmr_setclock },
  { "getclock", tmr_getclock },
  { NULL, NULL }
};

LUALIB_API int luaopen_tmr( lua_State *L )
{
  unsigned id;
  char name[ 10 ];
  
  luaL_register( L, AUXLIB_TMR, tmr_map );
  
  // Add all timers to our module
  for( id = 0; id < PLATFORM_TIMER_TOTAL; id ++ )
    if( platform_timer_exists( id ) )
    {
      sprintf( name, "TMR%d", id );
      lua_pushnumber( L, id );
      lua_setfield( L, -2, name );        
    }
  
  return 1;
}
