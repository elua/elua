#ifndef __MODCOMMON_H__
#define __MODCOMMON_H__ // Sentinel

// Const Table Entry
#define _ELUA_CTE(x) { #x, x }

// Const table typedef
typedef struct
{
  const char* name;
  u32 val;
} eLua_const_userdata_t;

#define _ELUA_MODTAB luaL_reg mod_##__COUNTER__[] __attribute__((section (".lua_init"))) 

// Register eLua module
int eLua_register(lua_State * l, const char * name, const luaL_Reg * t);
// Register const table
int eLua_register_const(lua_State * l, const eLua_const_userdata_t * ct);

#endif
