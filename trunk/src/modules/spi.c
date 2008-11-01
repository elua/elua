// Module for interfacing with Lua SPI code

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "auxmods.h"

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
static const luaL_reg spi_map[] = 
{
  { "setup",  spi_setup },
  { "select", spi_select },
  { "unselect", spi_unselect },
  { "send", spi_send },  
  { "send_recv", spi_send_recv },    
  { NULL, NULL }
};

LUALIB_API int luaopen_spi( lua_State *L )
{
  
  luaL_register( L, AUXLIB_SPI, spi_map );
  // Add the MASTER and SLAVE constants (for spi.setup)
  lua_pushnumber( L, PLATFORM_SPI_MASTER );
  lua_setfield( L, -2, "MASTER" );
  lua_pushnumber( L, PLATFORM_SPI_SLAVE );
  lua_setfield( L, -2, "SLAVE" );
  
  return 1;
}
