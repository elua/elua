// Module for interfacing with UART

//#include "lua.h"
//#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#//include "auxmods.h"
#include "lrotable.h"
#include "common.h"
//#include "sermux.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "platform_conf.h"

// Modes for the UART read function
enum
{
  UART_READ_MODE_LINE,
  UART_READ_MODE_NUMBER,
  UART_READ_MODE_SPACE,
  UART_READ_MODE_MAXSIZE
};

#define UART_INFINITE_TIMEOUT PLATFORM_TIMER_INF_TIMEOUT

// Helper function, the same as cmn_get_timeout_data() but with the
// parameters in the order required by the uart module.

static void uart_get_timeout_data( lua_State *L, int pidx, timer_data_type *ptimeout, unsigned *pid )
{
  lua_Number tempn;

  *ptimeout = PLATFORM_TIMER_INF_TIMEOUT;
  if( lua_type( L, pidx ) == LUA_TNUMBER )
  {
    tempn = lua_tonumber( L, pidx );
    if( tempn < 0 || tempn > PLATFORM_TIMER_INF_TIMEOUT )
      luaL_error( L, "invalid timeout value" );
    *ptimeout = ( timer_data_type )tempn;
  }
  *pid = ( unsigned )luaL_optinteger( L, pidx + 1, PLATFORM_TIMER_SYS_ID );
  if( *pid == PLATFORM_TIMER_SYS_ID && !platform_timer_sys_available() )
    luaL_error( L, "the system timer is not implemented on this platform" );
}


// Lua: actualbaud = setup( id, baud, databits, parity, stopbits )
static int uart_setup( lua_State* L )
{
  unsigned id, databits, parity, stopbits;
  u32 baud, res;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( uart, id );
  if( id >= SERMUX_SERVICE_ID_FIRST )
    return luaL_error( L, "uart.setup can't be called on virtual UARTs" );
  baud = luaL_checkinteger( L, 2 );
  databits = luaL_checkinteger( L, 3 );
  parity = luaL_checkinteger( L, 4 );
  stopbits = luaL_checkinteger( L, 5 );
  res = platform_uart_setup( id, baud, databits, parity, stopbits );
  lua_pushinteger( L, res );
  return 1;
}

// Lua: write( id, string1, [string2], ..., [stringn] )
static int uart_write( lua_State* L )
{
  int id;
  const char* buf;
  size_t len, i;
  int total = lua_gettop( L ), s;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( uart, id );
  for( s = 2; s <= total; s ++ )
  {
    if( lua_type( L, s ) == LUA_TNUMBER )
    {
      len = lua_tointeger( L, s );
      if( len > 255 )
        return luaL_error( L, "invalid number" );
      platform_uart_send( id, ( u8 )len );
    }
    else
    {
      luaL_checktype( L, s, LUA_TSTRING );
      buf = lua_tolstring( L, s, &len );
      for( i = 0; i < len; i ++ )
        platform_uart_send( id, buf[ i ] );
    }
  }
  return 0;
}

// Lua: uart.read( id, format, [timeout], [timer_id] )
static int uart_read( lua_State* L )
{
  int id, res, mode, issign;
  unsigned timer_id = PLATFORM_TIMER_SYS_ID;
  s32 maxsize = 0, count = 0;
  const char *fmt;
  luaL_Buffer b;
  char cres;
  timer_data_type timeout = PLATFORM_TIMER_INF_TIMEOUT;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( uart, id );

  // Check format
  if( lua_isnumber( L, 2 ) )
  {
    if( ( maxsize = ( s32 )lua_tointeger( L, 2 ) ) < 0 )
      return luaL_error( L, "invalid max size" );
    mode = UART_READ_MODE_MAXSIZE;
  }
  else
  {
    fmt = luaL_checkstring( L, 2 );
    if( !strcmp( fmt, "*l" ) )
      mode = UART_READ_MODE_LINE;
    else if( !strcmp( fmt, "*n" ) )
      mode = UART_READ_MODE_NUMBER;
    else if( !strcmp( fmt, "*s" ) )
      mode = UART_READ_MODE_SPACE;
    else
      return luaL_error( L, "invalid format" );
  }

  // Check timeout and timer id
  uart_get_timeout_data( L, 3, &timeout, &timer_id );

  // Read data
  luaL_buffinit( L, &b );
  while( 1 )
  {
	// TH: First try without timeout to avoid recv FIFO overflows because of timer overhead
	res=platform_uart_recv( id, timer_id, 0 );
	if ( res== -1 && timeout>0 )  res = platform_uart_recv( id, timer_id, timeout ); // Now try with timeout if one was given
    if( res  == -1 ) // still no char ...
      break; 
    cres = ( char )res;
    count ++;
    issign = ( count == 1 ) && ( ( res == '-' ) || ( res == '+' ) );
    // [TODO] this only works for lines that actually end with '\n', other line endings
    // are not supported.
    if( ( cres == '\n' ) && ( mode == UART_READ_MODE_LINE ) )
      break;
    if( !isdigit( (unsigned char) cres ) && !issign && ( mode == UART_READ_MODE_NUMBER ) )
      break;
    if( isspace( (unsigned char) cres ) && ( mode == UART_READ_MODE_SPACE ) )
      break;
    luaL_putchar( &b, cres );
    if( ( count == maxsize ) && ( mode == UART_READ_MODE_MAXSIZE ) )
      break;
  }
  luaL_pushresult( &b );

  // Return an integer if needed
  if( mode == UART_READ_MODE_NUMBER )
  {
    res = lua_tointeger( L, -1 );
    lua_pop( L, 1 );
    lua_pushinteger( L, res );
  }
  return 1;  
}

// Lua: data = getchar( id, [ timeout ], [ timer_id ] )
static int uart_getchar( lua_State* L )
{
  int id, res;
  char cres;
  unsigned timer_id = PLATFORM_TIMER_SYS_ID;
  timer_data_type timeout = PLATFORM_TIMER_INF_TIMEOUT;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( uart, id );
  // Check timeout and timer id
  uart_get_timeout_data( L, 2, &timeout, &timer_id );
  res = platform_uart_recv( id, timer_id, timeout );
  if( res == -1 )
    lua_pushstring( L, "" );
  else
  {
    cres = ( char )res;
    lua_pushlstring( L, &cres, 1 );
  }
  return 1;  
}


// Lua: uart.set_buffer( id, size )
static int uart_set_buffer( lua_State *L )
{
  int id = luaL_checkinteger( L, 1 );
  u32 size = ( u32 )luaL_checkinteger( L, 2 );
  
  MOD_CHECK_ID( uart, id );
  if( size && ( size & ( size - 1 ) ) )
    return luaL_error( L, "the buffer size must be a power of 2 or 0" );
  if( size == 0 && id >= SERMUX_SERVICE_ID_FIRST )
    return luaL_error( L, "disabling buffers on virtual UARTs is not allowed" );
  if( platform_uart_set_buffer( id, intlog2( size ) ) == PLATFORM_ERR )
    return luaL_error( L, "unable to set UART buffer" );
  return 0;
}

// Lua: uart.set_flow_control( id, type )
static int uart_set_flow_control( lua_State *L )
{
  int id = luaL_checkinteger( L, 1 );
  int type = luaL_checkinteger( L, 2 );

  MOD_CHECK_ID( uart, id );
  if( platform_uart_set_flow_control( id, type ) != PLATFORM_OK )
    return luaL_error( L, "unable to set the flow control on interface %d", id );
  return 0;
}

#if defined( BUILD_SERMUX ) || defined( BUILD_USB_CDC )

#define MAX_VUART_NAME_LEN    6
#define MIN_VUART_NAME_LEN    6

// __index metafunction for UART
// Look for all VUARTx timer identifiers
static int uart_mt_index( lua_State* L )
{
  const char *key = luaL_checkstring( L ,2 );
#ifdef BUILD_SERMUX
  char* pend;
  long res;
#endif

#ifdef BUILD_USB_CDC
  if( !strcmp( key, "CDC" ) )
  {
    lua_pushinteger( L, CDC_UART_ID );
    return 1;
  }
#endif
#ifdef BUILD_SERMUX
  if( strlen( key ) > MAX_VUART_NAME_LEN || strlen( key ) < MIN_VUART_NAME_LEN )
    return 0;
  if( strncmp( key, "VUART", 5 ) )
    return 0;  
  res = strtol( key + 5, &pend, 10 );
  if( *pend != '\0' )
    return 0;
  if( res >= SERMUX_NUM_VUART )
    return 0;
  lua_pushinteger( L, SERMUX_SERVICE_ID_FIRST + res );
  return 1;
#endif
  return 0;
}
#endif // #ifdef BUILD_SERMUX

// Module function map
#define MIN_OPT_LEVEL   2
#include "lrodefs.h"
const LUA_REG_TYPE uart_map[] = 
{
  { LSTRKEY( "setup" ),  LFUNCVAL( uart_setup ) },
  { LSTRKEY( "write" ), LFUNCVAL( uart_write ) },
  { LSTRKEY( "read" ), LFUNCVAL( uart_read ) },
  { LSTRKEY( "getchar" ), LFUNCVAL( uart_getchar ) },
  { LSTRKEY( "set_buffer" ), LFUNCVAL( uart_set_buffer ) },
  { LSTRKEY( "set_flow_control" ), LFUNCVAL( uart_set_flow_control ) },
#if LUA_OPTIMIZE_MEMORY > 0
  { LSTRKEY( "PAR_EVEN" ), LNUMVAL( PLATFORM_UART_PARITY_EVEN ) },
  { LSTRKEY( "PAR_ODD" ), LNUMVAL( PLATFORM_UART_PARITY_ODD ) },
  { LSTRKEY( "PAR_NONE" ), LNUMVAL( PLATFORM_UART_PARITY_NONE ) },
  { LSTRKEY( "PAR_MARK" ), LNUMVAL( PLATFORM_UART_PARITY_MARK ) },
  { LSTRKEY( "PAR_SPACE" ), LNUMVAL( PLATFORM_UART_PARITY_SPACE ) },
  { LSTRKEY( "STOP_1" ), LNUMVAL( PLATFORM_UART_STOPBITS_1 ) },
  { LSTRKEY( "STOP_1_5" ), LNUMVAL( PLATFORM_UART_STOPBITS_1_5 ) },
  { LSTRKEY( "STOP_2" ), LNUMVAL( PLATFORM_UART_STOPBITS_2 ) },
  { LSTRKEY( "NO_TIMEOUT" ), LNUMVAL( 0 ) },
  { LSTRKEY( "INF_TIMEOUT" ), LNUMVAL( UART_INFINITE_TIMEOUT ) },
  { LSTRKEY( "FLOW_NONE" ), LNUMVAL( PLATFORM_UART_FLOW_NONE ) },
  { LSTRKEY( "FLOW_RTS" ), LNUMVAL( PLATFORM_UART_FLOW_RTS ) },
  { LSTRKEY( "FLOW_CTS" ), LNUMVAL( PLATFORM_UART_FLOW_CTS ) },
#endif
#if LUA_OPTIMIZE_MEMORY > 0 && ( defined( BUILD_SERMUX ) || defined( BUILD_USB_CDC ) )
  { LSTRKEY( "__metatable" ), LROVAL( uart_map ) },
  { LSTRKEY( "__index" ), LFUNCVAL( uart_mt_index ) },  
#endif
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_uart( lua_State *L )
{
#if LUA_OPTIMIZE_MEMORY > 0
  return 0;
#else // #if LUA_OPTIMIZE_MEMORY > 0
  luaL_register( L, AUXLIB_UART, uart_map );
  
  MOD_REG_NUMBER( L, "PAR_EVEN", PLATFORM_UART_PARITY_EVEN );
  MOD_REG_NUMBER( L, "PAR_ODD", PLATFORM_UART_PARITY_ODD );
  MOD_REG_NUMBER( L, "PAR_NONE", PLATFORM_UART_PARITY_NONE );
  MOD_REG_NUMBER( L, "PAR_EVEN", PLATFORM_UART_PARITY_MARK );
  MOD_REG_NUMBER( L, "PAR_EVEN", PLATFORM_UART_PARITY_SPACE );
  MOD_REG_NUMBER( L, "STOP_1", PLATFORM_UART_STOPBITS_1 );
  MOD_REG_NUMBER( L, "STOP_1_5", PLATFORM_UART_STOPBITS_1_5 );
  MOD_REG_NUMBER( L, "STOP_2", PLATFORM_UART_STOPBITS_2 );
  MOD_REG_NUMBER( L, "NO_TIMEOUT", 0 );
  MOD_REG_NUMBER( L, "INF_TIMEOUT", UART_INFINITE_TIMEOUT );
  MOD_REG_NUMBER( L, "FLOW_NONE", PLATFORM_UART_FLOW_NONE );
  MOD_REG_NUMBER( L, "FLOW_RTS", PLATFORM_UART_FLOW_RTS );
  MOD_REG_NUMBER( L, "FLOW_CTS", PLATFORM_UART_FLOW_CTS );
  
  return 1;
#endif // #if LUA_OPTIMIZE_MEMORY > 0
}

