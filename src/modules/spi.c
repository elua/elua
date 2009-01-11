// Module for interfacing with Lua SPI code

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "auxmods.h"
#include "lrotable.h"

// Lua: select( id )
static int spi_select( lua_State* L )
{
  unsigned id;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( spi, id );
  platform_spi_select( id, PLATFORM_SPI_SELECT_ON );
  return 0;
}

// Lua: unselect( id )
static int spi_unselect( lua_State* L )
{
  unsigned id;
    
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( spi, id );
  platform_spi_select( id, PLATFORM_SPI_SELECT_OFF );
  return 0;
}

// Lua: setup( id, MASTER/SLAVE, clock, cpol, cpha, databits )
static int spi_setup( lua_State* L )
{
  unsigned id, cpol, cpha, is_master, databits;
  u32 clock, res;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( spi, id );
  is_master = luaL_checkinteger( L, 2 );
  clock = luaL_checkinteger( L, 3 );
  cpol = luaL_checkinteger( L, 4 );
  cpha = luaL_checkinteger( L, 5 );
  databits = luaL_checkinteger( L, 6 );
  res = platform_spi_setup( id, is_master, clock, cpol, cpha, databits );
  lua_pushinteger( L, res );
  return 1;
}

// Lua: send( id, out1, out2, ... )
static int spi_send( lua_State* L )
{
  spi_data_type value;
  int total = lua_gettop( L ), i, id;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( spi, id );
  for( i = 2; i <= total; i ++ )
  {
    value = luaL_checkinteger( L, i );  
    platform_spi_send_recv( id, value );
  }
  return 0;
}

// Lua: in1, in2, ... = send_recv( id, out1, out2, ... )
static int spi_send_recv( lua_State* L )
{
  spi_data_type value;
  int total = lua_gettop( L ), i, id;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( spi, id );
  for( i = 2; i <= total; i ++ )
  {
    value = luaL_checkinteger( L, i );  
    value = platform_spi_send_recv( id, value );
    lua_pushinteger( L, value );
  }
  return total - 1;  
}

// Module function map
#define MIN_OPT_LEVEL 2
#include "lrodefs.h"
const LUA_REG_TYPE spi_map[] = 
{
  { LSTRKEY( "setup" ),  LFUNCVAL( spi_setup ) },
  { LSTRKEY( "select" ),  LFUNCVAL( spi_select ) },
  { LSTRKEY( "unselect" ),  LFUNCVAL( spi_unselect ) },
  { LSTRKEY( "send" ),  LFUNCVAL( spi_send ) },  
  { LSTRKEY( "send_recv" ),  LFUNCVAL( spi_send_recv ) },    
#if LUA_OPTIMIZE_MEMORY > 0
  { LSTRKEY( "MASTER" ), LNUMVAL( PLATFORM_SPI_MASTER ) } ,
  { LSTRKEY( "SLAVE" ), LNUMVAL( PLATFORM_SPI_SLAVE ) },
#endif
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_spi( lua_State *L )
{
#if LUA_OPTIMIZE_MEMORY > 0
  return 0;
#else // #if LUA_OPTIMIZE_MEMORY > 0
  luaL_register( L, AUXLIB_SPI, spi_map );
  
  // Add the MASTER and SLAVE constants (for spi.setup)
  lua_pushnumber( L, PLATFORM_SPI_MASTER );
  lua_setfield( L, -2, "MASTER" );
  lua_pushnumber( L, PLATFORM_SPI_SLAVE );
  lua_setfield( L, -2, "SLAVE" );
  
  return 1;
#endif // #if LUA_OPTIMIZE_MEMORY > 0  
}
