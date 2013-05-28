// MBED specific PIO support
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


// mbed pin map from DIP pins to lpc17xx ports/pins
u32 pin_ports[] = { 0,0,0,0,0,0,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,1 ,2,2,2,2,2,2,0 ,0 ,0,0, // mbed p5 - p30 -- mapped to P0_
                 1 ,1 ,1 ,1 ,0,0 }; // mbed LED1-4, USBTX, USBRX -- mapped to P1_
u32 pin_nums[]  = { 9,8,7,6,0,1,18,17,15,16,23,24,25,26,30,31,5,4,3,2,1,0,11,10,5,4,
                 18,20,21,23,2,3 };

static int mbed_pio_mt_index( lua_State* L ) 
{
  const char *key = luaL_checkstring( L, 2 );
  int pin, sz;
  
  if( *key == 'P' || *key == 'p' ) // p5 - p30
  {
    if( sscanf( key + 1, "%d%n", &pin, &sz ) != 1  || sz != strlen( key ) - 1 )
      return 0;
    if(pin < 5 || pin > 30)
      return 0;
    pin -= 5;
  }
  else if ( *key == 'L' ) // LED1-4
  {
    pin = key[3] - '0';
    if ( pin > 4 || pin < 1 )
      return 0;
    pin += 25; // LED pins begin at pin_*[26], led #s begin at 1
  }
  else if ( *key == 'U' ) // USBTX, USBRX
  {
    switch( key[3] )
    {
      case 'T': pin = 30;
      case 'R': pin = 31;
      default: return 0;
    }
  }
  
  sz = PLATFORM_IO_ENCODE( pin_ports[ pin ], pin_nums[ pin ], 0 );

  if( sz == -1 )
    return 0;
  else
  {
    lua_pushinteger( L, sz );
    return 1;
  }
}

// Module function map
#define MIN_OPT_LEVEL 2
#include "lrodefs.h" 
const LUA_REG_TYPE mbed_pio_map[] =
{
#if LUA_OPTIMIZE_MEMORY > 0
  { LSTRKEY( "__metatable" ), LROVAL( mbed_pio_map ) },
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
  { LSTRKEY( "__index" ), LFUNCVAL( mbed_pio_mt_index ) },
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_mbed_pio( lua_State *L )
{
#if LUA_OPTIMIZE_MEMORY > 0
  return 0;
#else
  luaL_register( L, PS_LIB_TABLE_NAME, mbed_pio_map );
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
