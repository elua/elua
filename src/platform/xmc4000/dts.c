
// eLua module for XMC4500's die temperature sensor (DTS) module

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "lrotable.h"
#include "platform_conf.h"
#include "DAVE.h"

// Lua: xmc4000.dts.enable()
static int dts_enable(lua_State *L) {
  XMC_SCU_EnableTemperatureSensor();

  return 0;
}

// Lua: xmc4000.dts.disable()
static int dts_disable(lua_State *L) {
  XMC_SCU_DisableTemperatureSensor();

  return 0;
}

// Lua: xmc4000.dts.isenabled()
static int dts_is_enabled(lua_State *L) {
  lua_pushinteger( L, ( int )XMC_SCU_IsTemperatureSensorEnabled() );

  return 1;
}

// Lua: xmc4000.dts.isready()
static int dts_is_ready(lua_State *L) {
  lua_pushinteger( L, ( int )XMC_SCU_IsTemperatureSensorReady() );

  return 1;
}

// Lua: xmc4000.dts.start()
static int dts_start(lua_State *L) {
  XMC_SCU_StartTemperatureMeasurement();

  return 0;
}

// Lua: xmc4000.dts.measure()
static int dts_measure(lua_State *L) {
  lua_pushinteger( L, XMC_SCU_GetTemperatureMeasurement() );

  return 1;
}

// Lua: xmc4000.dts.isbusy()
static int dts_is_busy(lua_State *L) {
  lua_pushinteger( L, ( int )XMC_SCU_IsTemperatureSensorBusy() );

  return 1;
}

// Lua: xmc4000.dts.calibrate(offset, gain)
static int dts_calibrate(lua_State *L) {
  uint32_t offset, gain;

  offset = ( uint32_t )luaL_checkstring(L, 1);
  gain = ( uint32_t )luaL_checkstring(L, 2);
  XMC_SCU_CalibrateTemperatureSensor(offset, gain);

  return 0;
}

#define MIN_OPT_LEVEL 2
#include "lrodefs.h"

// Module function map
const LUA_REG_TYPE dts_map[] =
{
  { LSTRKEY( "enable" ),  LFUNCVAL( dts_enable ) },
  { LSTRKEY( "disable" ), LFUNCVAL( dts_disable ) },
  { LSTRKEY( "isenabled" ), LFUNCVAL( dts_is_enabled ) },
  { LSTRKEY( "isready" ), LFUNCVAL( dts_is_ready ) },
  { LSTRKEY( "start" ), LFUNCVAL( dts_start ) },
  { LSTRKEY( "measure" ), LFUNCVAL( dts_measure ) },
  { LSTRKEY( "isbusy" ), LFUNCVAL( dts_is_busy ) },
  { LSTRKEY( "calibrate" ), LFUNCVAL( dts_calibrate ) },
  { LNILKEY, LNILVAL }
};
