/* Read-only tables for Lua */

#ifndef lrotable_h
#define lrotable_h

#include "lua.h"
#include "llimits.h"
#include "lobject.h"
#include "luaconf.h"
#include "lgc.h"

/* Macros one can use to define rotable entries */
#define LRO_FUNCVAL(v)  {{.p = v}, LUA_TLIGHTFUNCTION}
#define LRO_NUMVAL(v)   {{.n = v}, LUA_TNUMBER}
#define LRO_ROVAL(v)    {{.p = ( void* )&v}, LUA_TROTABLE}
#define LRO_NILVAL      {{.p = NULL}, LUA_TNIL}

#define LRO_STRKEY(k)   {LUA_TSTRING, {.strkey = k}}
#define LRO_NUMKEY(k)   {LUA_TNUMBER, {.numkey = k}}
#define LRO_NILKEY      {LUA_TNIL, {.strkey=NULL}}

/* Maximum length of a rotable name and of a string key*/
#define LUA_MAX_ROTABLE_NAME      32

/* The 'LROTABLEBIT' is the same as the 'readonly bit' in lgc.h */
#define LROTABLEBIT     READONLYBIT          

/* Type of a numeric key in a rotable */
typedef int luaR_numkey;

/* The next structure defines the type of a key */
typedef struct
{
  int type;
  union
  {
    const char*   strkey;
    luaR_numkey   numkey;
  } id;
} luaR_key;

/* An entry in the read only table */
typedef struct
{
  const luaR_key key;
  const TValue value;
} luaR_entry;

/* A rotable */
typedef struct
{
  CommonHeader;
  const luaR_entry entries[];
} luaR_table;

/* The common header definition for a rotable */
#define LRO_HEADER      NULL, LUA_TROTABLE, ( 1 << LROTABLEBIT )

void* luaR_findglobal(const char *key, unsigned len);
int luaR_findfunction(lua_State *L, const luaR_table *ptable);
const TValue* luaR_findentry(void *data, const char *strkey, luaR_numkey numkey, unsigned *ppos);
void luaR_getcstr(char *dest, const TString *src, size_t maxsize);
void luaR_next(lua_State *L, void *data, TValue *key, TValue *val);
void* luaR_getmeta(void *data);
#ifdef LUA_META_ROTABLES
int luaR_isrotable(void *p);
#else
#define luaR_isrotable(p)     (0)
#endif

#endif
