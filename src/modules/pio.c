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
#define PIO_MAX_STRSIZE     16
#define PIO_MAX_TOKENSIZE   5

// Helper: scan to the next occurence of a given char and return
// the type found (string or number). The last token, number and
// type are held in the global variables pio_p_token, pio_p_num
// and pio_p_type. The function returns the next position in buffer
// (after delim) or NULL for error or empty string.

#define PIO_SCAN_NUMBER     0
#define PIO_SCAN_STRING     1
#define PIO_SCAN_ERROR      2

static char pio_p_token[ PIO_MAX_TOKENSIZE + 1 ];
static int pio_p_num;
static int pio_p_type;

const char* pio_scan_next( const char* src, int delim )
{
  int isnum = 1, cnt = 0;
  char *ptoken;

  pio_p_num = 0;
  ptoken = pio_p_token;
  pio_p_type = PIO_SCAN_ERROR;
  while( *src && *src != delim )
  {
    *ptoken ++ = *src;
    if( isnum && isdigit( *src ) )
      pio_p_num = ( pio_p_num << 3 ) + ( pio_p_num << 1 ) + *src - '0';
    else
      isnum = 0;
    src ++;
    if( ++cnt > PIO_MAX_TOKENSIZE )
      return NULL;
  }
  *ptoken = '\0';
  pio_p_type = isnum && cnt > 0 ? PIO_SCAN_NUMBER : PIO_SCAN_STRING;
  return src;
}

// Helper: get port and pin number from port string (PA_2, P1_3 ...)
// Also look for direction specifiers (PA_2_DIR) as well as pull
// specifiers(P1_3_PULL)
// Returns PIO_ERROR, PIO_PORT or PIO_PORT_AND_PIN
// Also returns port number and pin mask by side effect (and mode)
#define PIO_ERROR         0
#define PIO_PORT          1
#define PIO_PORT_AND_PIN  2
#define PIO_MODE_INOUT    0
#define PIO_MODE_DIR      1
#define PIO_MODE_PULL     2
static int pioh_parse_port( const char* key, int* pport, pio_type* ppinmask, int *pspin, int* pmode )
{
  int spin = -1, epin = -1, i;
  const char* p;

  *pmode = PIO_MODE_INOUT;
  if( !key || *key != 'P' || strlen( key ) > PIO_MAX_STRSIZE )
    return PIO_ERROR;
  
  // Get port first
  p = pio_scan_next( key + 1, '_' );
  if( pio_p_type == PIO_SCAN_ERROR )
    return PIO_ERROR;
  else if( pio_p_type == PIO_SCAN_NUMBER )
  {
    if( PIO_PREFIX != '0' )
      return PIO_ERROR;
    *pport = pio_p_num;
  }
  else
  {
    if( PIO_PREFIX != 'A' || strlen( pio_p_token ) != 1 || !isupper( pio_p_token[ 0 ] ) )
      return PIO_ERROR;
    *pport = pio_p_token[ 0 ] - 'A';
  }
  if( !platform_pio_has_port( *pport ) )
    return PIO_ERROR;
  if( *p == '\0' )
    return PIO_PORT;

  // Scan next three tokens
  // The first can be either a number or a pull/dir spec
  // The second can be either a number or a pull/dir spec
  // The last one can only be a pull/dir spec
  for( i = 0; i < 3; i ++ )
  {
    p = pio_scan_next( p + 1, '_' );
    if( pio_p_type == PIO_SCAN_ERROR )
      return PIO_ERROR;
    else if( pio_p_type == PIO_SCAN_STRING )
      goto handle_dir_pull;
    else switch( i ) // token is a number
    {
      case 0:
        spin = pio_p_num;
        break;

      case 1:
        epin = pio_p_num;
        break;

      case 2:
        return PIO_ERROR;
    }
    if( *p == '\0' ) // no more tokens
      goto finalize;
  }

  // Handle the "_DIR" and "_PULL" suffixes
handle_dir_pull:
  if( !strcmp( pio_p_token, "DIR" ) && *p == '\0' )
    *pmode = PIO_MODE_DIR;
  else if( !strcmp( pio_p_token, "PULL" ) && *p == '\0' )
    *pmode = PIO_MODE_PULL;
  else
    return PIO_ERROR;

finalize:
  // Build mask if needed
  if( spin != -1 )
  {
    if( !platform_pio_has_pin( *pport, spin ) || ( ( epin != -1 ) && !platform_pio_has_pin( *pport, epin ) ) )
      return PIO_ERROR;
    *pspin = spin;
    if( epin == -1 )
      *ppinmask = 1 << spin;
    else 
      if( epin < spin )
        return PIO_ERROR;
      else
        if( epin - spin + 1 == sizeof( pio_type ) << 3 )
          *ppinmask = PLATFORM_IO_ALL_PINS;
        else
          *ppinmask = ( ( 1 << ( epin - spin + 1 ) ) - 1 ) << spin;
    return PIO_PORT_AND_PIN;
  }
  return PIO_PORT;
}

// __index metafunction for PIO
// Return the value read from the given port/pin (nothing for error)
static int pio_mt_index( lua_State* L )
{
  const char *key = luaL_checkstring( L, 2 );
  int port, spin, res, mode;
  pio_type value, pinmask;

  if( ( res = pioh_parse_port( key, &port, &pinmask, &spin, &mode ) == PIO_ERROR ) )
    return 0;
  if( mode != PIO_MODE_INOUT )
    return 0;
  if( res == PIO_PORT )
    value = platform_pio_op( port, PLATFORM_IO_READ_IN_MASK, PLATFORM_IO_PORT_GET_VALUE );
  else
    if( pinmask & ( pinmask - 1 ) )
    {
      value = platform_pio_op( port, PLATFORM_IO_READ_IN_MASK, PLATFORM_IO_PORT_GET_VALUE );
      value = ( value & pinmask ) >> spin;
    }
    else
      value = platform_pio_op( port, pinmask, PLATFORM_IO_PIN_GET );
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
  int port, spin, res, mode;
  pio_type pinmask, temp;

  if( ( res = pioh_parse_port( key, &port, &pinmask, &spin, &mode ) == PIO_ERROR ) )
    return 0;
  switch( mode )
  {
    case PIO_MODE_INOUT: // set port/pin value
      if( res == PIO_PORT )
        platform_pio_op( port, value, PLATFORM_IO_PORT_SET_VALUE );
      else
      {
        temp = platform_pio_op( port, PLATFORM_IO_READ_OUT_MASK, PLATFORM_IO_PORT_GET_VALUE );
        value = ( temp & ~pinmask ) | ( value << spin );
        platform_pio_op( port, value, PLATFORM_IO_PORT_SET_VALUE );
      }
      break;
      
    case PIO_MODE_DIR: // set pin direction
      if( res == PIO_PORT )
        platform_pio_op( port, 0, value == PIO_DIR_INPUT ? PLATFORM_IO_PORT_DIR_INPUT : PLATFORM_IO_PORT_DIR_OUTPUT );
      else
        platform_pio_op( port, pinmask, value == PIO_DIR_INPUT ? PLATFORM_IO_PIN_DIR_INPUT : PLATFORM_IO_PIN_DIR_OUTPUT );
      break;

    case PIO_MODE_PULL: // pullup/pulldown configuration
      platform_pio_op( port, res == PIO_PORT ? PLATFORM_IO_ALL_PINS : pinmask, value ); 
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
  int port, spin, res, mode;
  pio_type pinmask;

  if( ( res = pioh_parse_port( key, &port, &pinmask, &spin, &mode ) == PIO_ERROR ) )
    return 0;
  if( mode != PIO_MODE_INOUT )
    return 0;
  if( res == PIO_PORT )
    platform_pio_op( port, 0, value == PIO_DIR_INPUT ? PLATFORM_IO_PORT_DIR_INPUT : PLATFORM_IO_PORT_DIR_OUTPUT );
  else
    platform_pio_op( port, pinmask, value == PIO_DIR_INPUT ? PLATFORM_IO_PIN_DIR_INPUT : PLATFORM_IO_PIN_DIR_OUTPUT );
  return 0;
}

// __newindex metafunction for pio.pull
// Set the pull type (pullup, pulldown, nopin) to the given ports/pins
static int pio_pull_mt_newindex( lua_State* L )
{
  const char* key = luaL_checkstring( L, 2 );
  pio_type value = ( pio_type )luaL_checkinteger( L, 3 );
  int port, spin, res, mode;
  pio_type pinmask;

  if( ( res = pioh_parse_port( key, &port, &pinmask, &spin, &mode ) == PIO_ERROR ) )
    return 0;
  if( mode != PIO_MODE_INOUT )
    return 0;
  platform_pio_op( port, res == PIO_PORT ? PLATFORM_IO_ALL_PINS : pinmask, value );
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

