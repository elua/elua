// Module for interfacing with PWM

//#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "auxmods.h"
#include "lrotable.h"

// Lua: realfrequency = setup( id, frequency, duty )
static int pwm_setup( lua_State* L )
{
  s32 freq;	  // signed, to error check for negative values
  unsigned duty;
  unsigned id;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( pwm, id );
  freq = luaL_checkinteger( L, 2 );
  if ( freq <= 0 )
    return luaL_error( L, "frequency must be > 0" );
  duty = luaL_checkinteger( L, 3 );
  if ( duty > 100 )
    // Negative values will turn out > 100, so will also fail.
    return luaL_error( L, "duty cycle must be from 0 to 100" );
  freq = platform_pwm_setup( id, (u32)freq, duty );
  lua_pushinteger( L, freq );
  return 1;  
}

// Lua: start( id )
static int pwm_start( lua_State* L )
{
  unsigned id;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( pwm, id );
  platform_pwm_start( id );
  return 0;  
}

// Lua: stop( id )
static int pwm_stop( lua_State* L )
{
  unsigned id;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( pwm, id );
  platform_pwm_stop( id );
  return 0;  
}

// Lua: realclock = setclock( id, clock )
static int pwm_setclock( lua_State* L )
{
  unsigned id;
  s32 clk;	// signed to error-check for negative values
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( pwm, id );
  clk = luaL_checkinteger( L, 2 );
  if ( clk <= 0 )
    return luaL_error( L, "frequency must be > 0" );
  clk = platform_pwm_set_clock( id, (u32)clk );
  lua_pushinteger( L, clk );
  return 1;
}

// Lua: clock = getclock( id )
static int pwm_getclock( lua_State* L )
{
  unsigned id;
  u32 clk;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( pwm, id );
  clk = platform_pwm_get_clock( id );
  lua_pushinteger( L, clk );
  return 1;
}

// Module function map
#define MIN_OPT_LEVEL 2
#include "lrodefs.h"
const LUA_REG_TYPE pwm_map[] = 
{
  { LSTRKEY( "setup" ), LFUNCVAL( pwm_setup ) },
  { LSTRKEY( "start" ), LFUNCVAL( pwm_start ) },
  { LSTRKEY( "stop" ), LFUNCVAL( pwm_stop ) },
  { LSTRKEY( "setclock" ), LFUNCVAL( pwm_setclock ) },
  { LSTRKEY( "getclock" ), LFUNCVAL( pwm_getclock ) },
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_pwm( lua_State *L )
{
  LREGISTER( L, AUXLIB_PWM, pwm_map );
}
