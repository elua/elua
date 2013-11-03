// Module for interfacing with Lua SPI code

//#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "auxmods.h"
#include "lrotable.h"

// Lua: sson( id )
static int spi_sson( lua_State* L )
{
  unsigned id;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( spi, id );
  platform_spi_select( id, PLATFORM_SPI_SELECT_ON );
  return 0;
}

// Lua: ssoff( id )
static int spi_ssoff( lua_State* L )
{
  unsigned id;
    
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( spi, id );
  platform_spi_select( id, PLATFORM_SPI_SELECT_OFF );
  return 0;
}

// Lua: clock = setup( id, MASTER/SLAVE, clock, cpol, cpha, databits )
static int spi_setup( lua_State* L )
{
  unsigned id, cpol, cpha, is_master, databits;
  u32 clock, res;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( spi, id );
  is_master = luaL_checkinteger( L, 2 ) == PLATFORM_SPI_MASTER;
  if( !is_master )
    return luaL_error( L, "invalid type (only spi.MASTER is supported)" );
  clock = luaL_checkinteger( L, 3 );
  cpol = luaL_checkinteger( L, 4 );
  if( ( cpol != 0 ) && ( cpol != 1 ) )
    return luaL_error( L, "invalid clock polarity." );
  cpha = luaL_checkinteger( L, 5 );
  if( ( cpha != 0 ) && ( cpha != 1 ) )
    return luaL_error( L, "invalid clock phase." );
  databits = luaL_checkinteger( L, 6 );
  res = platform_spi_setup( id, is_master, clock, cpol, cpha, databits );
  lua_pushinteger( L, res );
  return 1;
}

// Helper function: generic write/readwrite
static int spi_rw_helper( lua_State *L, int withread )
{
  spi_data_type value;
  const char *sval; 
  int total = lua_gettop( L ), i, j, id;
  size_t len, residx = 1;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( spi, id );
  if( withread )
    lua_newtable( L );
  for( i = 2; i <= total; i ++ )
  {
    if( lua_isnumber( L, i ) )
    {
      value = platform_spi_send_recv( id, lua_tointeger( L, i ) );
      if( withread )
      {
        lua_pushnumber( L, value );
        lua_rawseti( L, -2, residx ++ );
      }
    }
    else if( lua_isstring( L, i ) )
    {
      sval = lua_tolstring( L, i, &len );
      for( j = 0; j < len; j ++ )
      {
        value = platform_spi_send_recv( id, sval[ j ] );
        if( withread )
        {
          lua_pushnumber( L, value );
          lua_rawseti( L, -2, residx ++ );
        }
      }
    }
  }
  return withread ? 1 : 0;
}

// Lua: write( id, out1, out2, ... )
static int spi_write( lua_State* L )
{
  return spi_rw_helper( L, 0 );
}

// Lua: restable = readwrite( id, out1, out2, ... )
static int spi_readwrite( lua_State* L )
{
  return spi_rw_helper( L, 1 );
}

// Module function map
#define MIN_OPT_LEVEL 2
#include "lrodefs.h"
const LUA_REG_TYPE spi_map[] = 
{
  { LSTRKEY( "setup" ),  LFUNCVAL( spi_setup ) },
  { LSTRKEY( "sson" ),  LFUNCVAL( spi_sson ) },
  { LSTRKEY( "ssoff" ),  LFUNCVAL( spi_ssoff ) },
  { LSTRKEY( "write" ),  LFUNCVAL( spi_write ) },  
  { LSTRKEY( "readwrite" ),  LFUNCVAL( spi_readwrite ) },    
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
  MOD_REG_NUMBER( L, "MASTER", PLATFORM_SPI_MASTER );
  MOD_REG_NUMBER( L, "SLAVE", PLATFORM_SPI_SLAVE );  
  
  return 1;
#endif // #if LUA_OPTIMIZE_MEMORY > 0  
}
