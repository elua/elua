
// eLua module for Syncoam (SEPS525F) OLED display on the Infineon HMI
// hexagonal extension board
//
// Taken from platform/lm3s/disp.c. Adapted for Infineon XMC4500 by
// Raman

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
  { LNILKEY, LNILVAL }
};
