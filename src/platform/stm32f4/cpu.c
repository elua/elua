// eLua Module for STM32 cpu commands
// cpu is a platform-dependent (STM32) module, that binds to Lua the basic API
// from ST

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "lrotable.h"
#include "platform_conf.h"
#include "auxmods.h"
#include "cpu.h"

//Lua: reset()
static int cpu_reset( lua_State *L )
{
  stm32_cpu_reset();

  return 0;
}

#define MIN_OPT_LEVEL 2
#include "lrodefs.h"  

// Module function map
const LUA_REG_TYPE stm32_cpu_map[] =
{ 
  { LSTRKEY( "reset" ),  LFUNCVAL( cpu_reset ) },
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_stm32_cpu( lua_State *L )
{
  LREGISTER( L, AUXLIB_STM32_CPU, stm32_cpu_map );
}  

