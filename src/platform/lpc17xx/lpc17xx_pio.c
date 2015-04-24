// LPC17xx specific PIO support
#include <string.h>
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "lrotable.h"
#include "platform_conf.h"
#include "auxmods.h"
#include "lpc17xx_pinsel.h"

static int configpin( lua_State* L )
{
  pio_type v = ( pio_type )luaL_checkinteger( L, 1 );
  int funcnum = luaL_checkinteger( L, 2 );
  int opendrain = luaL_checkinteger( L, 3 );
  int pinmode = luaL_checkinteger( L, 4 );
  PINSEL_CFG_Type PinCfg;
  int port, pin;

  port = PLATFORM_IO_GET_PORT( v );
  pin = PLATFORM_IO_GET_PIN( v );
  if( PLATFORM_IO_IS_PORT( v ) || !platform_pio_has_port( port ) || !platform_pio_has_pin( port, pin ) )
    return luaL_error( L, "invalid pin" );
  
  PinCfg.Funcnum = funcnum;
  PinCfg.OpenDrain = opendrain;
  PinCfg.Pinmode = pinmode;
  PinCfg.Portnum = port;
  PinCfg.Pinnum = pin;
  PINSEL_ConfigPin(&PinCfg);
  
  return 0;
}

// Module function map
#define MIN_OPT_LEVEL 2
#include "lrodefs.h" 
const LUA_REG_TYPE lpc17xx_pio_map[] =
{
#if LUA_OPTIMIZE_MEMORY > 0
  { LSTRKEY( "__metatable" ), LROVAL( lpc17xx_pio_map ) },
  { LSTRKEY(  "RES_PULLUP" ), LNUMVAL( PINSEL_PINMODE_PULLUP )},
  { LSTRKEY(  "RES_TRISTATE" ), LNUMVAL( PINSEL_PINMODE_TRISTATE )},
  { LSTRKEY(  "RES_PULLDOWN" ), LNUMVAL( PINSEL_PINMODE_PULLDOWN )},
  { LSTRKEY(  "FUNCTION_0" ), LNUMVAL( PINSEL_FUNC_0 )},
  { LSTRKEY(  "FUNCTION_1" ), LNUMVAL( PINSEL_FUNC_1 )},
  { LSTRKEY(  "FUNCTION_2" ), LNUMVAL( PINSEL_FUNC_2 )},
  { LSTRKEY(  "FUNCTION_3" ), LNUMVAL( PINSEL_FUNC_3 )},
  { LSTRKEY(  "MODE_DEFAULT" ), LNUMVAL( PINSEL_PINMODE_NORMAL )},
  { LSTRKEY(  "MODE_OD" ), LNUMVAL( PINSEL_PINMODE_OPENDRAIN )},
#endif
  { LSTRKEY( "configpin" ),  LFUNCVAL( configpin ) },
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_lpc17xx_pio( lua_State *L )
{
#if LUA_OPTIMIZE_MEMORY > 0
  return 0;
#else
  luaL_register( L, PS_LIB_TABLE_NAME, lpc17xx_pio_map );
  MOD_REG_NUMBER( L, "RES_PULLUP", PINSEL_PINMODE_PULLUP );
  MOD_REG_NUMBER( L, "RES_TRISTATE", PINSEL_PINMODE_TRISTATE );
  MOD_REG_NUMBER( L, "RES_PULLDOWN", PINSEL_PINMODE_PULLDOWN );
  MOD_REG_NUMBER( L, "FUNCTION_0", PINSEL_FUNC_0 );
  MOD_REG_NUMBER( L, "FUNCTION_1", PINSEL_FUNC_1 );
  MOD_REG_NUMBER( L, "FUNCTION_2", PINSEL_FUNC_2 );
  MOD_REG_NUMBER( L, "FUNCTION_3", PINSEL_FUNC_3 );
  MOD_REG_NUMBER( L, "MODE_DEFAULT", PINSEL_PINMODE_NORMAL );
  MOD_REG_NUMBER( L, "MODE_OD", PINSEL_PINMODE_OPENDRAIN );
  
  // Set it as its own metatable
  lua_pushvalue( L, -1 );
  lua_setmetatable( L, -2 );
  
  return 1;  
#endif
}  
