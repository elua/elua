// Module for interfacing with Luminary Micro LM3S onboard goodies

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "auxmods.h"


//Lua: init(frequency)
static int disp_init(lua_State *L) {
  unsigned long freq;
  
  freq = luaL_checkinteger(L, 1);
  platform_disp_init(freq);
  return 0;
   
}

//Lua: enable
static int disp_enable(lua_State *L) {
  unsigned long freq;
  
  luaL_checkinteger(L, 1);
  platform_disp_enable(freq);
  return 0;
}

//Lua: disable   
static int disp_disable(lua_State *L) {    
  platform_disp_disable();
  return 0; 
}
   
//Lua: on
static int disp_on(lua_State *L) {
  platform_disp_displayOn();    
  return 0; 
}

//Lua: off
static int disp_off(lua_State *L) {    
  platform_disp_displayOff();
  return 0; 
}
   
//Lua: clear
static int disp_clear(lua_State *L) {    
  platform_disp_clear();
  return 0; 
}
   
//Lua: strDraw
static int disp_stringDraw(lua_State *L) {
  const char *str;
  unsigned long x; 
  unsigned long y;
  unsigned char lvl;
  
  str   = luaL_checkstring(L, 1);         
  x     = luaL_checkinteger(L, 2);
  y     = luaL_checkinteger(L, 3);
  lvl   = (unsigned char) luaL_checkinteger(L, 4);    
  platform_disp_stringDraw(str, x, y, lvl);
  return 0; 
}
   
//Lua: enable
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
  platform_disp_imageDraw(img, x, y, width, height);
  return 0; 
}
   
   
   
// Module function map
static const luaL_reg disp_map[] =
{
  { "init",  disp_init },
  { "enable",  disp_enable },
  { "disable", disp_disable },
  { "on", disp_on },    
  { "off", disp_off },
  { "clear", disp_clear },
  { "stringdraw", disp_stringDraw },
  { "imgagedraw", disp_imageDraw },  
  { NULL, NULL }
};

LUALIB_API int luaopen_disp( lua_State *L )
{
  luaL_register( L, AUXLIB_DISP, disp_map );
  return 1;
}  


