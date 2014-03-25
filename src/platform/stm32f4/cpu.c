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

#include "stm32f4xx.h"

//Lua: reset()
static int cpu_reset( lua_State *L )
{
  stm32_cpu_reset();

  return 0;
}

//Lua: freqtable = getclocksfreq()
static int cpu_getclocksfreq( lua_State *L )
{
  RCC_ClocksTypeDef rcc_clocks;
  RCC_GetClocksFreq( &rcc_clocks );
  lua_newtable( L );
  lua_pushinteger( L, rcc_clocks.SYSCLK_Frequency );
  lua_setfield( L , -2, "SYSCLK" );
  lua_pushinteger( L, rcc_clocks.HCLK_Frequency );
  lua_setfield( L , -2, "HCLK" );
  lua_pushinteger( L, rcc_clocks.PCLK1_Frequency );
  lua_setfield( L , -2, "PCLK1" );
  lua_pushinteger( L, rcc_clocks.PCLK2_Frequency );
  lua_setfield( L , -2, "PCLK2" );
  return 1;
}

//Lua: wfe()
static int cpu_wfe( lua_State *L )
{
  __WFE();
  return 0;
}

//Lua: wfi()
static int cpu_wfi( lua_State *L )
{
  __WFI();

  return 0;
}

//Lua: string = read_mcu_device_id()
static int cpu_read_mcu_device_id( lua_State *L )
{
  lua_pushnumber( L, *(uint32_t *)0xe0042000 );
  return 1;
}

//Lua: string = read_unique_device_id()
static int cpu_read_unique_device_id( lua_State *L )
{
  uint32_t *id_addr = (uint32_t *)0x1fff7a10;
  lua_newtable( L );
  int i;
  for( i = 1; i <= 3; ++i ) {
    lua_pushnumber( L, *id_addr++ );
    lua_rawseti( L, -2, i );
  }
  return 1;
}

static int rng_initialised = 0;

//Lua: rng.setup()
static int cpu_rng_setup( lua_State *L )
{
  RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_RNG, ENABLE);
  RNG_DeInit();
  RNG_ClearFlag( RNG_FLAG_CECS | RNG_FLAG_SECS );
  RNG_Cmd(ENABLE);
  rng_initialised = 1;

  return 0;
}

//Lua: num = rng.read()
static int cpu_rng_read( lua_State *L )
{
  if( !rng_initialised )
    cpu_rng_setup( L );

  while( RNG_GetFlagStatus( RNG_FLAG_DRDY ) == RESET ) {
    if( RNG_GetFlagStatus( RNG_FLAG_CECS ) == SET ||
        RNG_GetFlagStatus( RNG_FLAG_SECS ) == SET )
    {
      rng_initialised = 0;
      lua_pushnil( L );
      return 1;
    }
  }

  lua_pushnumber( L, RNG_GetRandomNumber() );
  return 1;
}

#define MIN_OPT_LEVEL 2
#include "lrodefs.h"  

const LUA_REG_TYPE stm32_cpu_rng_map[] =
{
  { LSTRKEY( "setup" ), LFUNCVAL( cpu_rng_setup ) },
  { LSTRKEY( "read" ), LFUNCVAL( cpu_rng_read ) },
  { LNILKEY, LNILVAL }
};

// Module function map
const LUA_REG_TYPE stm32_cpu_map[] =
{ 
  { LSTRKEY( "getclocksfreq" ),  LFUNCVAL( cpu_getclocksfreq ) },
  { LSTRKEY( "read_mcu_device_id" ), LFUNCVAL( cpu_read_mcu_device_id ) },
  { LSTRKEY( "read_unique_device_id" ), LFUNCVAL( cpu_read_unique_device_id ) },
  { LSTRKEY( "reset" ),  LFUNCVAL( cpu_reset ) },
  { LSTRKEY( "wfe" ),  LFUNCVAL( cpu_wfe ) },
  { LSTRKEY( "wfi" ),  LFUNCVAL( cpu_wfi ) },
  { LSTRKEY( "rng" ), LROVAL( stm32_cpu_rng_map ) },
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_stm32_cpu( lua_State *L )
{
  LREGISTER( L, AUXLIB_STM32_CPU, stm32_cpu_map );
}  

