// eLua Module for LM3S RIT OLED Display Support
// disp is a platform-dependent (LM3S) module, that binds to Lua the basic API
// from Luminary Micro

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "lrotable.h"
#include "platform_conf.h"
#include "disp.h"

//Lua: init(frequency)
static int disp_init(lua_State *L) {
  unsigned long freq;
  
  freq = luaL_checkinteger(L, 1);
  lm3s_disp_init(freq);
  return 0;
   
}

//Lua: enable(frequency)
static int disp_enable(lua_State *L) {
  unsigned long freq;
  
  freq = luaL_checkinteger(L, 1);
  lm3s_disp_enable(freq);
  return 0;
}

//Lua: disable()   
static int disp_disable(lua_State *L) {    
  lm3s_disp_disable();
  return 0; 
}
   
//Lua: on()
static int disp_on(lua_State *L) {
  lm3s_disp_displayOn();    
  return 0; 
}

//Lua: off()
static int disp_off(lua_State *L) {    
  lm3s_disp_displayOff();
  return 0; 
}
   
//Lua: clear()
static int disp_clear(lua_State *L) {    
  lm3s_disp_clear();
  return 0; 
}
   
//Lua: strDraw(str, x, y, lvl)
static int disp_stringDraw(lua_State *L) {
  const char *str;
  unsigned long x; 
  unsigned long y;
  unsigned char lvl;
  
  str   = luaL_checkstring(L, 1);         
  x     = luaL_checkinteger(L, 2);
  y     = luaL_checkinteger(L, 3);
  lvl   = (unsigned char) luaL_checkinteger(L, 4);    
  lm3s_disp_stringDraw(str, x, y, lvl);
  return 0; 
}
   
// Lua: imageDraw(img, x, y, width, height)
static int disp_imageDraw(lua_State *L) {    
  const char *img;
  unsigned long x;
  unsigned long y;
  unsigned long width;
  unsigned long height;
  
  img    = luaL_checkstring(L, 1);
  x      = luaL_checkinteger(L, 2);
  y      = luaL_checkinteger(L, 3);
  width  = luaL_checkinteger(L, 4);
  height = luaL_checkinteger(L, 5);
  lm3s_disp_imageDraw(( const unsigned char* )img, x, y, width, height);
  return 0; 
}   



#define MIN_OPT_LEVEL 2
#include "lrodefs.h"  

// Module function map
const LUA_REG_TYPE disp_map[] =
{ 
  { LSTRKEY( "init" ),  LFUNCVAL( disp_init ) },
  { LSTRKEY( "enable" ),  LFUNCVAL( disp_enable ) },
  { LSTRKEY( "disable" ), LFUNCVAL( disp_disable ) },
  { LSTRKEY( "on" ), LFUNCVAL( disp_on ) },    
  { LSTRKEY( "off" ), LFUNCVAL( disp_off ) },
  { LSTRKEY( "clear" ), LFUNCVAL( disp_clear ) },
  { LSTRKEY( "print" ), LFUNCVAL( disp_stringDraw ) },
  { LSTRKEY( "draw" ), LFUNCVAL( disp_imageDraw ) },  
  { LNILKEY, LNILVAL }
};
