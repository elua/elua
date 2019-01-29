
// eLua module for Syncoam (SEPS525F) OLED display on the Infineon HMI
// hexagonal extension board
//
// Taken from platform/lm3s/disp.c. Adapted for Infineon XMC4500 Hexagonal
// kit by Raman

#if defined( ELUA_BOARD_XMC4500HEXAGON )

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "lrotable.h"
#include "platform_conf.h"
#include "disp.h"
#include "DAVE.h"

// Lua: init()
static int disp_init(lua_State *L) {
  GUI_SEGGERLIBRARY_Init(&GUI_SEGGERLIBRARY_0);
  GUI_SetFont(&GUI_Font16B_1);
  return 0;   
}

// Lua: clear()
static int disp_clear(lua_State *L) {    
  GUI_Clear();
  return 0; 
}

// Lua: strDraw(str, x, y)
static int disp_stringDraw(lua_State *L) {
  const char *str;
  unsigned long x; 
  unsigned long y;
  unsigned long color;
  
  str   = luaL_checkstring(L, 1);         
  x     = luaL_checkinteger(L, 2);
  y     = luaL_checkinteger(L, 3);
  color = luaL_checkinteger(L, 4);

  GUI_SetColor(color);
  GUI_DispStringAt(str, x, y);
  return 0; 
}
   
#define MIN_OPT_LEVEL 2
#include "lrodefs.h"  

// Module function map
const LUA_REG_TYPE disp_map[] =
{ 
  { LSTRKEY( "init" ),  LFUNCVAL( disp_init ) },
  { LSTRKEY( "clear" ), LFUNCVAL( disp_clear ) },
  { LSTRKEY( "print" ), LFUNCVAL( disp_stringDraw ) },
  /* Expose colors from GUI.h */
  { LSTRKEY( "GUI_BLUE" ), LNUMVAL( GUI_BLUE ) },
  { LSTRKEY( "GUI_GREEN" ), LNUMVAL( GUI_GREEN ) },
  { LSTRKEY( "GUI_RED" ), LNUMVAL( GUI_RED ) },
  { LSTRKEY( "GUI_CYAN" ), LNUMVAL( GUI_CYAN ) },
  { LSTRKEY( "GUI_MAGENTA" ), LNUMVAL( GUI_MAGENTA ) },
  { LSTRKEY( "GUI_YELLOW" ), LNUMVAL( GUI_YELLOW ) },
  { LSTRKEY( "GUI_LIGHTBLUE" ), LNUMVAL( GUI_LIGHTBLUE ) },
  { LSTRKEY( "GUI_LIGHTGREEN" ), LNUMVAL( GUI_LIGHTGREEN ) },
  { LSTRKEY( "GUI_LIGHTRED" ), LNUMVAL( GUI_LIGHTRED ) },
  { LSTRKEY( "GUI_LIGHTCYAN" ), LNUMVAL( GUI_LIGHTCYAN ) },
  { LSTRKEY( "GUI_LIGHTMAGENTA" ), LNUMVAL( GUI_LIGHTMAGENTA ) },
  { LSTRKEY( "GUI_LIGHTYELLOW" ), LNUMVAL( GUI_LIGHTYELLOW ) },
  { LSTRKEY( "GUI_DARKBLUE" ), LNUMVAL( GUI_DARKBLUE ) },
  { LSTRKEY( "GUI_DARKRED" ), LNUMVAL( GUI_DARKRED ) },
  { LSTRKEY( "GUI_DARKCYAN" ), LNUMVAL( GUI_DARKCYAN ) },
  { LSTRKEY( "GUI_DARKMAGENTA" ), LNUMVAL( GUI_DARKMAGENTA ) },
  { LSTRKEY( "GUI_DARKYELLOW" ), LNUMVAL( GUI_DARKYELLOW ) },
  { LSTRKEY( "GUI_WHITE" ), LNUMVAL( GUI_WHITE ) },
  { LSTRKEY( "GUI_LIGHTGRAY" ), LNUMVAL( GUI_LIGHTGRAY ) },
  { LSTRKEY( "GUI_GRAY" ), LNUMVAL( GUI_GRAY ) },
  { LSTRKEY( "GUI_DARKGRAY" ), LNUMVAL( GUI_DARKGRAY ) },
  { LSTRKEY( "GUI_BLACK" ), LNUMVAL( GUI_BLACK ) },
  { LSTRKEY( "GUI_BROWN" ), LNUMVAL( GUI_BROWN ) },
  { LSTRKEY( "GUI_ORANGE" ), LNUMVAL( GUI_ORANGE ) },
  { LSTRKEY( "GUI_TRANSPARENT" ), LNUMVAL( GUI_TRANSPARENT ) },
  { LNILKEY, LNILVAL }
};

#endif
