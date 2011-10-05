// STR9 specific PIO support

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "lrotable.h"
#include "platform_conf.h"
#include "91x_gpio.h"
#include "auxmods.h"

#define GPIO_DIR_INPUT      GPIO_PinInput
#define GPIO_DIR_OUTPUT     GPIO_PinOutput
#define GPIO_ALT_INPUT      GPIO_InputAlt1
#define GPIO_ALT_OUTPUT1    GPIO_OutputAlt1
#define GPIO_ALT_OUTPUT2    GPIO_OutputAlt2
#define GPIO_ALT_OUTPUT3    GPIO_OutputAlt3
#define GPIO_OUTPUT_PP      GPIO_Type_PushPull
#define GPIO_OUTPUT_OC      GPIO_Type_OpenCollector

// Puin mappings (for GPIO lib)
static const unsigned str9_pins[] = { GPIO_Pin_0, GPIO_Pin_1, GPIO_Pin_2, GPIO_Pin_3, GPIO_Pin_4, GPIO_Pin_5, GPIO_Pin_6, GPIO_Pin_7 };
extern const GPIO_TypeDef* port_data[];

// Lua: str9.pio.set_input( pin, direction, type, ipconnected, alternate )
// direction: INPUT, OUTPUT
// type: OUTPUT_PUSHPULL, OUTPUT_OC
// ipconnected: true or false
// alternate: ALT_INPUT, ALT_OUTPUT1, ALT_OUTPUT2, ALT_OUTPUT3
static int setpin( lua_State *L )
{
  pio_type v = ( pio_type )luaL_checkinteger( L, 1 );
  int direction = luaL_checkinteger( L, 2 );
  int type = luaL_checkinteger( L, 3 );
  int ipconnected = lua_toboolean( L, 4 );
  int alternate = luaL_checkinteger( L, 5 );
  GPIO_InitTypeDef GPIO_InitStructure;
  int port, pin;

  port = PLATFORM_IO_GET_PORT( v );
  pin = PLATFORM_IO_GET_PIN( v );
  if( PLATFORM_IO_IS_PORT( v ) || !platform_pio_has_port( port ) || !platform_pio_has_pin( port, pin ) )
    return luaL_error( L, "invalid pin" );

  GPIO_StructInit( &GPIO_InitStructure );  
  GPIO_InitStructure.GPIO_Pin = str9_pins[ pin ];
  GPIO_InitStructure.GPIO_Direction = direction;
  GPIO_InitStructure.GPIO_Type = type;
  GPIO_InitStructure.GPIO_IPConnected = ipconnected ? GPIO_IPConnected_Enable : GPIO_IPConnected_Disable;
  GPIO_InitStructure.GPIO_Alternate = alternate;
  GPIO_Init( ( GPIO_TypeDef* )port_data[ port ], &GPIO_InitStructure );
 
  return 0;
}

// Module function map
#define MIN_OPT_LEVEL 2
#include "lrodefs.h" 
const LUA_REG_TYPE str9_pio_map[] =
{
#if LUA_OPTIMIZE_MEMORY > 0
  { LSTRKEY( "INPUT" ), LNUMVAL( GPIO_DIR_INPUT ) },
  { LSTRKEY( "OUTPUT" ), LNUMVAL( GPIO_DIR_OUTPUT ) },
  { LSTRKEY( "ALT_INPUT" ), LNUMVAL( GPIO_ALT_INPUT ) },
  { LSTRKEY( "ALT_OUTPUT1" ), LNUMVAL( GPIO_ALT_OUTPUT1 ) },
  { LSTRKEY( "ALT_OUTPUT2" ), LNUMVAL( GPIO_ALT_OUTPUT2 ) },
  { LSTRKEY( "ALT_OUTPUT3" ), LNUMVAL( GPIO_ALT_OUTPUT3 ) },
  { LSTRKEY( "OUTPUT_PUSHPULL" ), LNUMVAL( GPIO_OUTPUT_PP ) },
  { LSTRKEY( "OUTPUT_OC" ), LNUMVAL( GPIO_OUTPUT_OC ) },
#endif
  { LSTRKEY( "setpin" ),  LFUNCVAL( setpin) },
  { LNILKEY, LNILVAL }
};
