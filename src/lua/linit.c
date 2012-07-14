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

// Dummy open function for "co" (actually opened in lbaselib.c)
int luaopen_co( lua_State *L )
{
  return 0;
}

#define _ROM( name, openf, table ) { name, openf },

static const luaL_Reg lualibs[] = {
  {"", luaopen_base},
#ifdef LUA_PLATFORM_LIBS_REG
  LUA_PLATFORM_LIBS_REG,
#endif 
#if defined(LUA_PLATFORM_LIBS_ROM)
  LUA_PLATFORM_LIBS_ROM
#endif
#if defined(LUA_LIBS_NOLTR)
  LUA_LIBS_NOLTR
#endif
  {NULL, NULL}
};

#if defined(LUA_PLATFORM_LIBS_ROM) && LUA_OPTIMIZE_MEMORY == 2
#undef _ROM
#define _ROM( name, openf, table ) extern const luaR_entry table[];
LUA_PLATFORM_LIBS_ROM;
#endif
const luaR_table lua_rotable[] = 
{
#if defined(LUA_PLATFORM_LIBS_ROM) && LUA_OPTIMIZE_MEMORY == 2
#undef _ROM
#define _ROM( name, openf, table ) { name, table },
  LUA_PLATFORM_LIBS_ROM
#endif
  {NULL, NULL}
};

LUALIB_API void luaL_openlibs (lua_State *L) {
  const luaL_Reg *lib = lualibs;
  for (; lib->func; lib++) {
    lua_pushcfunction(L, lib->func);
    lua_pushstring(L, lib->name);
    lua_call(L, 1, 0);
  }
}

