// Module for interfacing Lua code with a Controller Area Network (CAN)

//#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "auxmods.h"
#include "lrotable.h"

// Lua: result = setup( id, clock )
static int can_setup( lua_State* L )
{
  unsigned id;
  u32 clock, res;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( can, id );
  clock = luaL_checkinteger( L, 2 );
  res = platform_can_setup( id, clock );
  lua_pushinteger( L, res );
  return 1;
}

// Lua: success = send( id, canid, canidtype, message )
static int can_send( lua_State* L )
{
  size_t len;
  int id, canid, idtype;
  const char *data;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( can, id );
  canid = luaL_checkinteger( L, 2 );
  idtype = luaL_checkinteger( L, 3 );
  data = luaL_checklstring (L, 4, &len);
  if ( len > PLATFORM_CAN_MAXLEN )
    return luaL_error( L, "message exceeds max length" );
  
  if( platform_can_send( id, canid, idtype, len, ( const u8 * )data ) == PLATFORM_OK )
    lua_pushboolean( L, 1 );
  else
    lua_pushboolean( L, 0 );
  
  return 1;
}

// Lua: canid, canidtype, message = recv( id )
static int can_recv( lua_State* L )
{
  u8 len;
  int id;
  u32 canid;
  u8  idtype, data[ PLATFORM_CAN_MAXLEN ];
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( can, id );
  
  if( platform_can_recv( id, &canid, &idtype, &len, data ) == PLATFORM_OK )
  {
    lua_pushinteger( L, canid );
    lua_pushinteger( L, idtype );
    lua_pushlstring( L, ( const char * )data, ( size_t )len );
  
    return 3;
  }
  else
    return 0;
}


// Module function map
#define MIN_OPT_LEVEL 2
#include "lrodefs.h"
const LUA_REG_TYPE can_map[] = 
{
  { LSTRKEY( "setup" ),  LFUNCVAL( can_setup ) },
  { LSTRKEY( "send" ),  LFUNCVAL( can_send ) },  
  { LSTRKEY( "recv" ),  LFUNCVAL( can_recv ) },
#if LUA_OPTIMIZE_MEMORY > 0
  { LSTRKEY( "ID_STD" ), LNUMVAL( ELUA_CAN_ID_STD ) },
  { LSTRKEY( "ID_EXT" ), LNUMVAL( ELUA_CAN_ID_EXT ) },
#endif
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_can( lua_State *L )
{
#if LUA_OPTIMIZE_MEMORY > 0
  return 0;
#else // #if LUA_OPTIMIZE_MEMORY > 0
  luaL_register( L, AUXLIB_CAN, can_map );
  
  // Module constants  
  MOD_REG_NUMBER( L, "ID_STD", ELUA_CAN_ID_STD );
  MOD_REG_NUMBER( L, "ID_EXT", ELUA_CAN_ID_EXT );
  
  return 1;
#endif // #if LUA_OPTIMIZE_MEMORY > 0  
}
