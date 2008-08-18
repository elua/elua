// Module for interfacing with PWM

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "auxmods.h"

// Lua: realfrequency = setup( id, frequency, duty )
static int pwm_setup( lua_State* L )
{
  u32 freq;
  unsigned duty, id;
  
  MOD_CHECK_MIN_ARGS( 3 );  
  id = luaL_checkinteger( L, 1 );
  freq = luaL_checkinteger( L, 2 );
  duty = luaL_checkinteger( L, 3 );
  if( duty > 100 )
    duty = 100;
  freq = platform_pwm_setup( id, freq, duty );
  lua_pushinteger( L, freq );
  return 1;  
}

// Lua: start( id )
static int pwm_start( lua_State* L )
{
  unsigned id;
  
  MOD_CHECK_MIN_ARGS( 1 );
  id = luaL_checkinteger( L, 1 );
  platform_pwm_op( id, PLATFORM_PWM_OP_START, 0 );
  return 0;  
}

// Lua: stop( id )
static int pwm_stop( lua_State* L )
{
  unsigned id;
  
  MOD_CHECK_MIN_ARGS( 1 );
  id = luaL_checkinteger( L, 1 );
  platform_pwm_op( id, PLATFORM_PWM_OP_STOP, 0 );
  return 0;  
}

// Lua: realclock = setclock( id, clock )
static int pwm_setclock( lua_State* L )
{
  unsigned id;
  u32 clk;
  
  MOD_CHECK_MIN_ARGS( 2 );
  id = luaL_checkinteger( L, 1 );
  clk = luaL_checkinteger( L, 2 );
  clk = platform_pwm_op( id, PLATFORM_PWM_OP_SET_CLOCK, clk );
  lua_pushinteger( L, clk );
  return 1;
}

// Lua: clock = getclock( id )
static int pwm_getclock( lua_State* L )
{
  unsigned id;
  u32 clk;
  
  MOD_CHECK_MIN_ARGS( 1 );
  id = luaL_checkinteger( L, 1 );
  clk = platform_pwm_op( id, PLATFORM_PWM_OP_GET_CLOCK, 0 );
  lua_pushinteger( L, clk );
  return 1;
}

// Module function map
static const luaL_reg pwm_map[] = 
{
  { "setup", pwm_setup },
  { "start", pwm_start },
  { "stop", pwm_stop },
  { "setclock", pwm_setclock },
  { "getclock", pwm_getclock },
  { NULL, NULL }
};

LUALIB_API int luaopen_pwm( lua_State *L )
{
  unsigned id;
  char name[ 10 ];
  
  luaL_register( L, AUXLIB_PWM, pwm_map );
  
  // Add all PWM interfaces to our module
  for( id = 0; id < PLATFORM_PWM_TOTAL; id ++ )
    if( platform_pwm_exists( id ) )
    {
      sprintf( name, "PWM%d", id );
      lua_pushnumber( L, id );
      lua_setfield( L, -2, name );        
    }
    
  return 1;
}
