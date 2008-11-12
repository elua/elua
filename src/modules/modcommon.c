#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "auxmods.h"
#include <string.h> 

#include "modcommon.h"

#define MODCOMMON_CT_NAME "__const_table_"

static int const_mt_index(lua_State * L)
{
	const char * key = luaL_checkstring(L, 2);
	const eLua_const_userdata_t * ct;

	// Get the const table from the module table
	lua_pushliteral(L, MODCOMMON_CT_NAME);
	lua_rawget(L, 1);
	ct = (eLua_const_userdata_t *)lua_touserdata(L, -1);

	if (ct != NULL)
	{
		int i;

		for (i = 0; ct[i].name != NULL; i++)
		{
			if (strcmp(ct[i].name, key) == 0) // Match
			{
				lua_pushinteger(L, ct[i].val);
				return 1;
			}
		}
		return 0;
	}

	luaL_error(L, "Could not find constants table!");

	return 0; // Quiet warning
}

int eLua_register(lua_State * L, const char * name, const luaL_Reg * t)
{
	// Register methods
	luaL_register(L, name, t);

	return 1;
}

int eLua_register_const(lua_State * L, const eLua_const_userdata_t * ct)
{

	// Metatable data
	static const luaL_reg const_mt_map[] =
	{
	  { "__index", const_mt_index },
	  { NULL, NULL }
	};
	
	// Store the const table in the module table for later use.
	lua_pushliteral(L, MODCOMMON_CT_NAME);
	lua_pushlightuserdata(L, (eLua_const_userdata_t *)ct);
	lua_rawset(L, -3);
	
	lua_newtable(L);
	luaL_register(L, NULL, const_mt_map);
	lua_setmetatable(L, -2);

	return 1;
}

extern luaL_reg _lua_init_start;
extern luaL_reg _lua_init_end;

LUALIB_API int luaopen_mod(lua_State * L)
{
	luaL_reg * modtab = &_lua_init_start;

	printf("luaopen_mod(%p): lua_init_start = %p, lua_init_end = %p\n", L, &_lua_init_start, &_lua_init_end);

	printf("luaopen_mod(%p): Setting up package.preload for modules:\n", L);
	
	lua_getfield(L, LUA_GLOBALSINDEX, "package"); // Get the package table
	lua_getfield(L, -1, "preload");               // package.preload is now on stack top

	for ( ; modtab->func; modtab++)
	{
		printf("\t%s\n", modtab->name);
		lua_pushcfunction(L, modtab->func);
		lua_setfield(L, -2, modtab->name);
	}

	lua_pop(L, 2); // Restore stack position

	return 0;
}
