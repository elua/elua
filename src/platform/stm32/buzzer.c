
// Buzzer eLua module for elua-zeisig-gemacht target platform

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "lrotable.h"
#include "platform_conf.h"
#include "auxmods.h"
#include "stm32f10x.h"

//Lua: init()
static int buzzer_init( lua_State *L )
{

  GPIO_InitTypeDef bp;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

  /* Buzzer configuration */
  bp.GPIO_Pin = GPIO_Pin_4;
  bp.GPIO_Speed = GPIO_Speed_50MHz;
  bp.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOC, &bp);
  return 0;
}

//Lua: on()
static int buzzer_enable( lua_State *L )
{
  GPIO_SetBits(GPIOC, GPIO_Pin_4);
  return 0;
}

//Lua: off()
static int buzzer_disable( lua_State *L )
{
  GPIO_ResetBits(GPIOC, GPIO_Pin_4);
  return 0;
}

#define MIN_OPT_LEVEL 2
#include "lrodefs.h"  

// Module function map
const LUA_REG_TYPE buzzer_map[] =
{ 
  { LSTRKEY( "init" ),  LFUNCVAL( buzzer_init ) },
  { LSTRKEY( "on" ),  LFUNCVAL( buzzer_enable ) },
  { LSTRKEY( "off" ),  LFUNCVAL( buzzer_disable ) },
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_buzzer( lua_State *L )
{
  LREGISTER( L, AUXLIB_BUZZER, buzzer_map );
}  

