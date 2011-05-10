// eLua Module for Kinetis touch sensing interface
// platform-dependent (Kinetis) module, that binds to Lua to a basic API to
// control the touch-sensitive interface on kinetis

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "lrotable.h"
#include "platform_conf.h"
#include "auxmods.h"
#include "tsi.h"

//Lua: init(id)
static int tsi_init( lua_State *L )
{
  unsigned id;
  
  id = luaL_checkinteger( L, 1 );
  lua_pushinteger( L, kin_tsi_read( id ) );
  return 1;
}

//Lua: setcounter(id, count)
static int tsi_read( lua_State *L )
{
  unsigned id, baseline;
  u8 i, cnt = 0;

  id = luaL_checkinteger( L, 1 );
  baseline = luaL_checkinteger( L, 2 );
  
  for( i = 0; i < KIN_TSI_DBOUNCE_COUNTS; i++ )
  {
    if( kin_tsi_read( id ) >= baseline + KIN_TSI_TOUCH_LEVEL )
      cnt++;
  }
  if( cnt == KIN_TSI_DBOUNCE_COUNTS )
    lua_pushinteger( L, 1 );
  else
    lua_pushinteger( L, 0 );

  return 1;
}


#define MIN_OPT_LEVEL 2
#include "lrodefs.h"  

// Module function map
const LUA_REG_TYPE tsi_map[] =
{ 
  { LSTRKEY( "init" ),  LFUNCVAL( tsi_init ) },
  { LSTRKEY( "read" ),  LFUNCVAL( tsi_read ) },
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_enc( lua_State *L )
{
  LREGISTER( L, AUXLIB_ENC, tsi_map );
}  

