/*
** $Id: linit.c,v 1.1.1.1 2008/07/11 13:11:55 bogdanm Exp $
** Initialization of libraries for lua.c
** See Copyright Notice in lua.h
*/


#define linit_c
#define LUA_LIB

#include "lua.h"

#include "lualib.h"
#include "lauxlib.h"

#include "platform_libs.h"

static const luaL_Reg lualibs[] = {
  {"", luaopen_base},
  {LUA_LOADLIBNAME, luaopen_package},
  {LUA_TABLIBNAME, luaopen_table},
  {LUA_IOLIBNAME, luaopen_io},
  {LUA_OSLIBNAME, luaopen_os},
  {LUA_STRLIBNAME, luaopen_string},
#if !defined LUA_NUMBER_INTEGRAL  
  {LUA_MATHLIBNAME, luaopen_math},
#endif
  {LUA_DBLIBNAME, luaopen_debug},
#ifdef LUA_PLATFORM_LIBS  
  LUA_PLATFORM_LIBS,
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

