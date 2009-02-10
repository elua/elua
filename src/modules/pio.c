// Module for interfacing with PIO

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "auxmods.h"
#include "lrotable.h"
#include "platform_conf.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>

// PIO constants
#define PIO_DIR_OUTPUT      0
#define PIO_DIR_INPUT       1
#define PIO_PULLUP          0
#define PIO_PULLDOWN        1
#define PIO_NOPULL          2

// Helper: get port and pin number from port string (PA_2, P1_3 ...)
// Also look for direction specifiers (PA_2_DIR) as well as pull
// specifiers(P1_3_PULL)
// Returns PIO_ERROR, PIO_PORT or PIO_PORT_AND_PIN
// Also returns port and pin number by side effect (and mode)
#define PIO_ERROR         0
#define PIO_PORT          1
#define PIO_PORT_AND_PIN  2
#define PIO_MODE_INOUT    0
#define PIO_MODE_DIR      1
#define PIO_MODE_PULL     2
static int pioh_parse_port( const char* key, int* pport, int *ppin, int* pmode )
{
  int isport = 0, sz;
  int postsize = 0;
  const char* p;

  *pport = *ppin = -1;
  *pmode = PIO_MODE_INOUT;
  if( !key || *key != 'P' )
    return PIO_ERROR;
  // Look for suffix _DIR or _PULL
  if( strlen( key ) > 4 && !strcmp( key + strlen( key ) - 4, "_DIR" ) )
  {
    *pmode = PIO_MODE_DIR;
    postsize = 4;
  }
  else if( strlen( key ) > 5 && !strcmp( key + strlen( key ) - 5, "_PULL" ) )
  {
    *pmode = PIO_MODE_PULL;
    postsize = 5;
  }

  // Get port and (optionally) pin number
  if( isupper( key[ 1 ] ) ) // PA, PB, ...
  {
    if( PIO_PREFIX != 'A' )
      return PIO_ERROR;
    *pport = key[ 1 ] - 'A';
    if( strlen( key ) == 2 + postsize ) 
      isport = 1;
    else if( key[ 2 ] == '_' )      
      if( sscanf( key + 3, "%d%n", ppin, &sz ) != 1 || sz != strlen( key ) - 3 - postsize )
        return PIO_ERROR;      
  }
  else // P0, P1, ...
  {
    if( PIO_PREFIX != '0' )
      return PIO_ERROR;
    p = strchr( key, '_' );
    if( ( p == NULL ) || ( p == key + strlen( key ) - postsize ) )
    {
      if( sscanf( key + 1, "%d%n", pport, &sz ) != 1  || sz != strlen( key ) - 1 - postsize )
        return PIO_ERROR;
      isport = 1;
    }
    else    // parse port_pin
      if( sscanf( key + 1, "%d_%d%n", pport, ppin, &sz ) != 2 || sz != strlen( key ) - 1 - postsize )
        return PIO_ERROR;
  }
  return isport ? PIO_PORT : PIO_PORT_AND_PIN;
}

// __index metafunction for PIO
// Return the value read from the given port/pin (nothing for error)
static int pio_mt_index( lua_State* L )
{
  const char *key = luaL_checkstring( L, 2 );
  int port, pin, res, mode;
  pio_type value;

  if( ( res = pioh_parse_port( key, &port, &pin, &mode ) == PIO_ERROR ) )
    return 0;
  if( ( ( res == PIO_PORT ) && !platform_pio_has_port( port ) ) ||
      ( ( res == PIO_PORT_AND_PIN ) && !platform_pio_has_pin( port, pin ) ) )
    return 0; 
  if( mode != PIO_MODE_INOUT )
    return 0;
  if( res == PIO_PORT )
    value = platform_pio_op( port, PLATFORM_IO_ALL_PINS, PLATFORM_IO_PORT_GET_VALUE );
  else
    value = platform_pio_op( port, 1 << pin, PLATFORM_IO_PIN_GET );
  lua_pushinteger( L, value );
  return 1;
}

// __newindex metafunction for PIO
// Set the given value to the give port/pin, or the direction of the pin, or
// the pullup/pulldown configuration
static int pio_mt_newindex( lua_State* L )
{
  const char *key = luaL_checkstring( L, 2 );
  pio_type value = ( pio_type )luaL_checkinteger( L, 3 );
  int port, pin, res, mode;

  if( ( res = pioh_parse_port( key, &port, &pin, &mode ) == PIO_ERROR ) )
    return 0;
  if( ( ( res == PIO_PORT ) && !platform_pio_has_port( port ) ) ||
      ( ( res == PIO_PORT_AND_PIN ) && !platform_pio_has_pin( port, pin ) ) )
    return 0;
  switch( mode )
  {
    case PIO_MODE_INOUT: // set port/pin value
      if( res == PIO_PORT )
        platform_pio_op( port, value, PLATFORM_IO_PORT_SET_VALUE );
      else
        platform_pio_op( port, 1 << pin, value ? PLATFORM_IO_PIN_SET : PLATFORM_IO_PIN_CLEAR );
      break;
      
    case PIO_MODE_DIR: // set pin direction
      if( res == PIO_PORT )
        platform_pio_op( port, 0, value == PIO_DIR_INPUT ? PLATFORM_IO_PORT_DIR_INPUT : PLATFORM_IO_PORT_DIR_OUTPUT );
      else
        platform_pio_op( port, 1 << pin, value == PIO_DIR_INPUT ? PLATFORM_IO_PIN_DIR_INPUT : PLATFORM_IO_PIN_DIR_OUTPUT );
      break;

    case PIO_MODE_PULL: // pullup/pulldown configuration
      platform_pio_op( port, res == PIO_PORT ? PLATFORM_IO_ALL_PINS : 1 << pin, value ); 
      break;
  }
  return 0;
}

// __newindex metafunction for pio.dir
// Set the direction of the given ports/pins
static int pio_dir_mt_newindex( lua_State* L )
{
  const char* key = luaL_checkstring( L, 2 );
  pio_type value = ( pio_type )luaL_checkinteger( L, 3 );
  int port, pin, res, mode;

  if( ( res = pioh_parse_port( key, &port, &pin, &mode ) == PIO_ERROR ) )
    return 0;
  if( ( ( res == PIO_PORT ) && !platform_pio_has_port( port ) ) ||
      ( ( res == PIO_PORT_AND_PIN ) && !platform_pio_has_pin( port, pin ) ) )
    return 0;
  if( mode != PIO_MODE_INOUT )
    return 0;
  if( res == PIO_PORT )
    platform_pio_op( port, 0, value == PIO_DIR_INPUT ? PLATFORM_IO_PORT_DIR_INPUT : PLATFORM_IO_PORT_DIR_OUTPUT );
  else
    platform_pio_op( port, 1 << pin, value == PIO_DIR_INPUT ? PLATFORM_IO_PIN_DIR_INPUT : PLATFORM_IO_PIN_DIR_OUTPUT );
  return 0;
}

// __newindex metafunction for pio.pull
// Set the pull type (pullup, pulldown, nopin) to the given ports/pins
static int pio_pull_mt_newindex( lua_State* L )
{
  const char* key = luaL_checkstring( L, 2 );
  pio_type value = ( pio_type )luaL_checkinteger( L, 3 );
  int port, pin, res, mode;

  if( ( res = pioh_parse_port( key, &port, &pin, &mode ) == PIO_ERROR ) )
    return 0;
  if( ( ( res == PIO_PORT ) && !platform_pio_has_port( port ) ) ||
      ( ( res == PIO_PORT_AND_PIN ) && !platform_pio_has_pin( port, pin ) ) )
    return 0;
  if( mode != PIO_MODE_INOUT )
    return 0;
  platform_pio_op( port, res == PIO_PORT ? PLATFORM_IO_ALL_PINS : 1 << pin, value );
  return 0;
}

// Module function map
#define MIN_OPT_LEVEL 2
#include "lrodefs.h"
static const LUA_REG_TYPE pio_dir_map[] =
{
#if LUA_OPTIMIZE_MEMORY > 0
  { LSTRKEY( "__metatable" ), LROVAL( pio_dir_map ) },
#endif
  { LSTRKEY( "__newindex" ), LFUNCVAL( pio_dir_mt_newindex ) },
  { LNILKEY, LNILVAL }
};

static const LUA_REG_TYPE pio_pull_map[] =
{
#if LUA_OPTIMIZE_MEMORY > 0
  { LSTRKEY( "__metatable" ), LROVAL( pio_pull_map ) },
#endif
  { LSTRKEY( "__newindex" ), LFUNCVAL( pio_pull_mt_newindex ) },
  { LNILKEY, LNILVAL }
};

const LUA_REG_TYPE pio_map[] = 
{
#if LUA_OPTIMIZE_MEMORY > 0
  { LSTRKEY( "__metatable" ), LROVAL( pio_map ) },
  { LSTRKEY( "INPUT" ), LNUMVAL( PIO_DIR_INPUT ) },
  { LSTRKEY( "OUTPUT" ), LNUMVAL( PIO_DIR_OUTPUT ) },
  { LSTRKEY( "PULLUP" ), LNUMVAL( PLATFORM_IO_PIN_PULLUP ) },
  { LSTRKEY( "PULLDOWN" ), LNUMVAL( PLATFORM_IO_PIN_PULLDOWN ) },
  { LSTRKEY( "NOPULL" ), LNUMVAL( PLATFORM_IO_PIN_NOPULL ) },
  { LSTRKEY( "dir" ), LROVAL( pio_dir_map ) },
  { LSTRKEY( "pull" ), LROVAL( pio_pull_map ) },
#endif
  { LSTRKEY( "__index" ), LFUNCVAL( pio_mt_index ) },
  { LSTRKEY( "__newindex" ), LFUNCVAL( pio_mt_newindex ) },
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_pio( lua_State *L )
{
#if LUA_OPTIMIZE_MEMORY > 0
  return 0;
#else // #if LUA_OPTIMIZE_MEMORY > 0
  luaL_register( L, AUXLIB_PIO, pio_map );
  
  // Set this table as its own metatable
  lua_pushvalue( L, -1 );
  lua_setmetatable( L, -2 );

  // Set constants for direction/pullups
  lua_pushnumber( L, PIO_DIR_INPUT );
  lua_setfield( L, -2, "INPUT" );
  lua_pushnumber( L, PIO_DIR_OUTPUT );
  lua_setfield( L, -2, "OUTPUT" );
  lua_pushnumber( L, PLATFORM_IO_PIN_PULLUP );
  lua_setfield( L, -2, "PULLUP" );
  lua_pushnumber( L, PLATFORM_IO_PIN_PULLDOWN );
  lua_setfield( L, -2, "PULLDOWN" );
  lua_pushnumber( L, PLATFORM_IO_PIN_NOPULL );
  lua_setfield( L, -2, "NOPULL" );

  // Setup the new tables (dir and pull) inside pio
  lua_newtable( L );
  luaL_register( L, NULL, pio_dir_map );
  lua_pushvalue( L, -1 );
  lua_setmetatable( L, -2 );
  lua_setfield( L, -2, "dir" );

  lua_newtable( L );
  luaL_register( L, NULL, pio_pull_map );
  lua_pushvalue( L, -1 );
  lua_setmetatable( L, -2 );
  lua_setfield( L, -2, "pull" );

  return 1;
#endif // #if LUA_OPTIMIZE_MEMORY > 0
}

