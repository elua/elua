// MBED specific low power support
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "lrotable.h"
#include "platform_conf.h"
#include "auxmods.h"
#include "LPC17xx.h"

static int mbed_power_sleep( lua_State *L )
{  
  // SCB->SCR |= 0x04; // Deep Sleep
  __WFI();

  return 0;
}

// Module function map
#define MIN_OPT_LEVEL 2
#include "lrodefs.h" 
const LUA_REG_TYPE mbed_power_map[] =
{
  { LSTRKEY( "sleep" ),  LFUNCVAL( mbed_power_sleep ) },
  { LNILKEY, LNILVAL }
};

