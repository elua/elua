// Example module that shows how to use the rom-loader in modcommon.c

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "auxmods.h"
#include "modcommon.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define THENUMBERONE 1 // Example constant to demo _ELUA_CTE macro use

static int afunc(lua_State * L)
{
	lua_pushliteral(L, "afunc() called!");

	return 1;
}

static const eLua_const_userdata_t consts[] =
{
	_ELUA_CTE(THENUMBERONE),
	{ NULL, 0 }
};

static const luaL_reg funcs[] =
{
	{ "afunc", afunc },
	{ NULL, NULL }
}; // Pretty boring module ...

// Function that gets called when lua registers the module
LUALIB_API int luaopen_stm32(lua_State * L)
{
	eLua_register(L, "stm32", funcs);  // Register module methods first
	eLua_register_const(L, consts);    // Then register constants, if any.

	return 1;
}

// Add the module to the module table. Do *NOT* terminate this with a NULL entry
// The linker script does it for you.
_ELUA_MODTAB = {
	{ "stm32", luaopen_stm32 }
};

