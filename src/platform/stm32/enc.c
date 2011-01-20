// eLua Module for STM32 timer encoder mode support
// enc is a platform-dependent (STM32) module, that binds to Lua the basic API
// from ST

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "lrotable.h"
#include "platform_conf.h"
#include "auxmods.h"
#include "enc.h"

//Lua: init(id)
static int enc_init(lua_State *L) {
  unsigned id;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( timer, id );
  stm32_enc_init( id );
  return 0;
}

//Lua: set_counter(id, count)
static int enc_set_counter(lua_State *L) {
  unsigned id, count;

  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( timer, id );
  count = luaL_checkinteger( L, 2 );

  stm32_enc_set_counter( id, count );
  return 0;
}

#define MIN_OPT_LEVEL 2
#include "lrodefs.h"  

// Module function map
const LUA_REG_TYPE enc_map[] =
{ 
  { LSTRKEY( "init" ),  LFUNCVAL( enc_init ) },
  { LSTRKEY( "setcounter" ),  LFUNCVAL( enc_set_counter ) },
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_enc( lua_State *L )
{
  LREGISTER( L, AUXLIB_ENC, enc_map );
}  

