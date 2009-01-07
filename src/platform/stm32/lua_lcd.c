// Module to interface with the LCD on the STM3210E board.

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "auxmods.h"
#include "modcommon.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>

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

static const eLua_const_userdata_t lcd_constants[] = 
{
	_ELUA_CTE(Line0),
	_ELUA_CTE(Line1),
	_ELUA_CTE(Line2),
	_ELUA_CTE(Line3),
	_ELUA_CTE(Line4),
	_ELUA_CTE(Line5),
	_ELUA_CTE(Line6),
	_ELUA_CTE(Line7),
	_ELUA_CTE(Line8),
	_ELUA_CTE(Line9),
	
	{ NULL, 0 }
};

static const luaL_reg lcd_map[] =
{
	{ "init", lcd_init },
	{ "setforecolor", lcd_setforecolor },
	{ "setbackcolor", lcd_setbackcolor },
	{ "clear", lcd_clear },
	{ "clearline", lcd_clearline },
	{ "print", lcd_print },

	{ NULL, NULL }
};

//LUALIB_API int luaopen_lcd(lua_State * L) __attribute__ ((section (".lua_init")));
LUALIB_API int luaopen_lcd(lua_State * L)
{
	eLua_register(L, "stm3210lcd", lcd_map);
	eLua_register_const(L, lcd_constants);

	return 1;
}

const luaL_reg lcd_modtab[] __attribute__ ((section (".lua_init"))) = {
	{ "stm3210lcd", luaopen_lcd }
};

#endif

