
/* Signal capture module for eLua */

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "lrotable.h"
#include "platform_conf.h"
#include "DAVE.h"

// Lua: period_nano_secs, duty_cycle = xmc4000.capture.capture()
static int xmc_capture( lua_State *L )
{
  uint32_t pwm_period = 0, duty_cycle = 0;

  CAPTURE_Start( &CAPTURE_0 );
  CAPTURE_GetPeriodInNanoSecDutyCycleInPercentage( &CAPTURE_0, &pwm_period, &duty_cycle );
  lua_pushnumber( L, ( 1 / ( pwm_period * 1e-9 ) ) );  // period
  lua_pushinteger( L, ( lua_Integer ) duty_cycle );  // duty
  return 2;
}

#define MIN_OPT_LEVEL 2
#include "lrodefs.h"  

// Module function map
const LUA_REG_TYPE capture_map[] =
{
  { LSTRKEY( "capture" ),  LFUNCVAL( xmc_capture ) },
  { LNILKEY, LNILVAL }
};
