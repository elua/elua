/*
** $Id: linit.c,v 1.14.1.1 2007/12/27 13:02:25 roberto Exp $
** Initialization of libraries for lua.c
** See Copyright Notice in lua.h
*/


#define linit_c
#define LUA_LIB

#include "lua.h"

#include "lualib.h"
#include "lauxlib.h"
#include "lrotable.h"
#include "luaconf.h"
#include "platform_conf.h"

extern int luaopen_platform( lua_State *L );

static const luaL_Reg lualibs[] = {
  {"", luaopen_base},
  {LUA_LOADLIBNAME, luaopen_package},
  {LUA_IOLIBNAME, luaopen_io},
  {LUA_STRLIBNAME, luaopen_string},    
#if LUA_OPTIMIZE_MEMORY == 0
  {LUA_MATHLIBNAME, luaopen_math},
  {LUA_TABLIBNAME, luaopen_table},  
  {LUA_DBLIBNAME, luaopen_debug},  
#endif
#ifdef LUA_PLATFORM_LIBS_REG
  LUA_PLATFORM_LIBS_REG,
#endif 
#if defined(LUA_PLATFORM_LIBS_ROM)
#define _ROM( name, openf, table ) { name, openf },
  LUA_PLATFORM_LIBS_ROM
#endif
  {NULL, NULL}
};

extern const luaR_table strlib;
extern const luaR_table syslib;
extern const luaR_table tab_funcs;
extern const luaR_table dblib;
extern const luaR_table co_funcs;
#if defined(LUA_PLATFORM_LIBS_ROM) && LUA_OPTIMIZE_MEMORY == 2
#undef _ROM
#define _ROM( name, openf, table ) extern const luaR_table table;
LUA_PLATFORM_LIBS_ROM;
#endif

#define MIN_OPT_LEVEL 2
#include "lrodefs.h"
LHEADER( lua_rotables )
#if LUA_OPTIMIZE_MEMORY > 0
  {LSTRKEY(LUA_STRLIBNAME), LROVAL(strlib)},
  {LSTRKEY(LUA_TABLIBNAME), LROVAL(tab_funcs)},
  {LSTRKEY(LUA_DBLIBNAME), LROVAL(dblib)},
  {LSTRKEY(LUA_COLIBNAME), LROVAL(co_funcs)},
#if defined(LUA_PLATFORM_LIBS_ROM) && LUA_OPTIMIZE_MEMORY == 2
#undef _ROM
#define _ROM( name, openf, table ) { LSTRKEY(name), LROVAL(table) },
  LUA_PLATFORM_LIBS_ROM
#endif
#endif
  {LSTRKEY("_R"), LROVAL(lua_rotables)},
  {LNILKEY, LNILVAL}
LFOOTER

LUALIB_API void luaL_openlibs (lua_State *L) {
  const luaL_Reg *lib = lualibs;
  for (; lib->func; lib++) {
    lua_pushcfunction(L, lib->func);
    lua_pushstring(L, lib->name);
    lua_call(L, 1, 0);
  }
}

