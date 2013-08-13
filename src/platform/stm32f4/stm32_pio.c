// STM32 specific PIO support

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "lrotable.h"
#include "platform_conf.h"
#include "auxmods.h"
#include "stm32f4xx_conf.h"
#include <string.h>

#if LUA_OPTIMIZE_MEMORY == 0
#error stm32.pio can only be compiled with LTR on (optram=true)
#endif

#define PIN_FUNC_GPIO         ( -1 )
#define PIN_FUNC_ADC          ( -2 )
#define PIN_FUNC_ALT_FIRST    0
#define PIN_FUNC_ALT_LAST     15

extern GPIO_TypeDef * const pio_port[];

// Lua: stm32.pio.set_function( function, pin1, pin2, ..., pinn )
// Function can be stm32.pio.GPIO, stm32.pio.AFx (x = 0..15), stm32.pio.ADC
static int stm32_pio_set_function( lua_State *L )
{
  int func = luaL_checkinteger( L, 1 ), gpio, pin, port, idx;
  int total = lua_gettop( L ), i;
  GPIO_TypeDef *base;

  if( total == 1 )
    return luaL_error( L, "no pin specified" );
  for( i = 2; i <= total; i ++ )
  {
    gpio = luaL_checkinteger( L, i );
    port = PLATFORM_IO_GET_PORT( gpio );
    pin = PLATFORM_IO_GET_PIN( gpio );
    base = pio_port[ port ];
    if( func == PIN_FUNC_GPIO )
      base->MODER &= ~( 0x03 << ( pin * 2 ) );
    else if( func == PIN_FUNC_ADC )
      base->MODER |= 0x03 << ( pin * 2 );
    else if( func >= PIN_FUNC_ALT_FIRST && func <= PIN_FUNC_ALT_LAST )
    {
      base->MODER &= ~( 0x03 << ( pin * 2 ) );
      base->MODER |= 0x02 << ( pin * 2 );
      if( pin >= 8 )
      {
        pin -= 8;
        idx = 1;
      }
      else
        idx = 0;
      base->AFR[ idx ] &= ~( 0x0F << ( pin * 4 ) );
      base->AFR[ idx ] |= func << ( pin * 4 );
    }
  }
  return 0;
}

// Module function map
#define MIN_OPT_LEVEL 2
#include "lrodefs.h" 
const LUA_REG_TYPE stm32_pio_map[] =
{
  { LSTRKEY( "set_function" ), LFUNCVAL( stm32_pio_set_function ) },
  { LSTRKEY( "GPIO" ), LNUMVAL( PIN_FUNC_GPIO ) },
  { LSTRKEY( "ADC" ), LNUMVAL( PIN_FUNC_ADC ) },
  { LSTRKEY( "AF0" ), LNUMVAL( 0 ) },
  { LSTRKEY( "AF1" ), LNUMVAL( 1 ) },
  { LSTRKEY( "AF2" ), LNUMVAL( 2 ) },
  { LSTRKEY( "AF3" ), LNUMVAL( 3 ) },
  { LSTRKEY( "AF4" ), LNUMVAL( 4 ) },
  { LSTRKEY( "AF5" ), LNUMVAL( 5 ) },
  { LSTRKEY( "AF6" ), LNUMVAL( 6 ) },
  { LSTRKEY( "AF7" ), LNUMVAL( 7 ) },
  { LSTRKEY( "AF8" ), LNUMVAL( 8 ) },
  { LSTRKEY( "AF9" ), LNUMVAL( 9 ) },
  { LSTRKEY( "AF10" ), LNUMVAL( 10 ) },
  { LSTRKEY( "AF11" ), LNUMVAL( 11 ) },
  { LSTRKEY( "AF12" ), LNUMVAL( 12 ) },
  { LSTRKEY( "AF13" ), LNUMVAL( 13 ) },
  { LSTRKEY( "AF14" ), LNUMVAL( 14 ) },
  { LSTRKEY( "AF15" ), LNUMVAL( 15 ) },
  { LNILKEY, LNILVAL }
};

