// Module to interface with the LCD on the STM3210E board.

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "auxmods.h"
#include "modcommon.h"
#include "lrotable.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "platform_conf.h"
#include "lcd.h"

#ifdef FORSTM3210E_EVAL

static int lcd_init(lua_State * L)
{
  STM3210E_LCD_Init();

  return 0;
}

static int lcd_setforecolor(lua_State * L)
{
  u16 color = luaL_checkint(L, 1);

  LCD_SetTextColor(color);

  return 0;
}

static int lcd_setbackcolor(lua_State * L)
{
  u16 color = luaL_checkint(L, 1);

  LCD_SetBackColor(color);

  return 0;
}

static int lcd_clear(lua_State * L)
{
  u16 color;

  if (lua_gettop(L) == 0)
    color = 0x0000;
  else
    color = luaL_checkint(L, 1);

  LCD_Clear(color);

  return 0;
}

static int lcd_clearline(lua_State * L)
{
  u8 line = luaL_checkint(L, 1);

  LCD_ClearLine(line);

  return 0;
}

static int lcd_print(lua_State * L)
{
  u8   line = luaL_checkint(L, 1);
  u8 * text = (u8 *)luaL_checkstring(L, 2);

  LCD_DisplayStringLine(line, text);

  return 0;
}

static int lcd_mt_index( lua_State *L )
{
  const char *key = luaL_checkstring( L ,2 );
  int linedata[] = { Line0, Line1, Line2, Line3, Line4, Line5, Line6, Line7, Line8, Line9 };
  
  if( strlen( key ) != 5 || strncmp( key, "Line", 4 ) || !isdigit( key[ 4 ] ) ) 
    return 0;
  lua_pushinteger( L, linedata[ key[ 4 ] - '0' ] );
  return 1;
}

#define MIN_OPT_LEVEL 2
#include "lrodefs.h"
const LUA_REG_TYPE lcd_map[] =
{
  { LSTRKEY( "init" ), LFUNCVAL( lcd_init ) },
  { LSTRKEY( "setforecolor" ), LFUNCVAL( lcd_setforecolor ) },
  { LSTRKEY( "setbackcolor" ), LFUNCVAL( lcd_setbackcolor ) },
  { LSTRKEY( "clear" ), LFUNCVAL( lcd_clear ) },
  { LSTRKEY( "clearline" ), LFUNCVAL( lcd_clearline ) },
  { LSTRKEY( "print" ), LFUNCVAL( lcd_print ) },
#if LUA_OPTIMIZE_MEMORY > 0
  { LSTRKEY( "__metatable" ), LROVAL( lcd_map ) },
#endif
  { LSTRKEY( "__index" ), LFUNCVAL( lcd_mt_index ) },
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_lcd(lua_State * L)
{
#if LUA_OPTIMIZE_MEMORY > 0
  return 0;
#else // #if LUA_OPTIMIZE_MEMORY > 0
  luaL_register( L, AUXLIB_LCD, lcd_map );
  
  // Set this table as its own metatable
  lua_pushvalue( L, -1 );
  lua_setmetatable( L, -2 );
  
  return 1;
#endif // #if LUA_OPTIMIZE_MEMORY > 0  
}

#endif // #ifdef FORSTM3210E_EVAL
