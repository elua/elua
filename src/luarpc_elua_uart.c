#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "platform_conf.h"
#include "luarpc_rpc.h"

#if defined( BUILD_RPC )

// Buffer for async dispatch
int adispatch_buff = -1;

void set_adispatch_buff( int i );
int transport_get_char(Transport *t);

void set_adispatch_buff( int i )
{
  adispatch_buff = i;
}

// Setup Transport
void transport_init( Transport *tpt )
{
  tpt->fd = INVALID_TRANSPORT;
  tpt->tmr_id = 0;
}

// Read a char from serial buffer
int transport_get_char(Transport *t)
{
  return platform_uart_recv( t->fd, t->tmr_id, 0 );
}

// Open Listener / Server
void transport_open_listener( lua_State *L, ServerHandle *handle )
{
  // Get args & Set up connection
  unsigned uart_id, tmr_id;
  
  check_num_args( L,3 ); // 1st arg is uart num, 2nd arg is tmr_id, 3nd is handle
  if ( !lua_isnumber( L, 1 ) ) 
    luaL_error( L, "1st arg must be uart num" );
    
  if ( !lua_isnumber( L, 2 ) ) 
    luaL_error( L, "2nd arg must be timer num" );

  // @@@ Error handling could likely be better here
  uart_id = lua_tonumber( L, 1 );
  if( !platform_uart_exists( uart_id ) )
    luaL_error( L, "invalid uart id" );
  
  tmr_id = lua_tonumber( L, 2 );
  if( !platform_timer_exists( tmr_id ) )
    luaL_error( L, "invalid timer id" );
  
  handle->ltpt.fd = ( int )uart_id;
  handle->ltpt.tmr_id = tmr_id;

  // Setup uart
  platform_uart_setup( (unsigned int) uart_id, 115200, 8, PLATFORM_UART_PARITY_NONE, PLATFORM_UART_STOPBITS_1 );
}

// Open Connection / Client
int transport_open_connection(lua_State *L, Handle *handle)
{
  // Get args & Set up connection
  unsigned uart_id, tmr_id;
  
  check_num_args( L,3 ); // 1st arg is uart num, 2nd arg is tmr_id, 3nd is handle
  if ( !lua_isnumber( L, 1 ) ) 
    return luaL_error( L, "1st arg must be uart num" );
    
  if ( !lua_isnumber( L, 2 ) ) 
    return luaL_error( L, "2nd arg must be timer num" );

  uart_id = lua_tonumber( L, 1 );
  MOD_CHECK_ID( uart, uart_id );
  
  tmr_id = lua_tonumber( L, 1 );
  MOD_CHECK_ID( timer, tmr_id );
  
  adispatch_buff = -1;

  handle->tpt.fd = ( int )uart_id;
  handle->tpt.tmr_id = tmr_id;

  // Setup uart
  platform_uart_setup( (unsigned int) uart_id, 115200, 8, PLATFORM_UART_PARITY_NONE, PLATFORM_UART_STOPBITS_1 );

  return 1;
}

// Accept Connection 
void transport_accept( Transport *tpt, Transport *atpt )
{
  struct exception e;
  TRANSPORT_VERIFY_OPEN;
  atpt->fd = tpt->fd;
}

void transport_read_buffer( Transport *tpt, u8 *buffer, int length )
{
  int n = 0;
  int c;
  struct exception e;
  timer_data_type uart_timeout = PLATFORM_TIMER_INF_TIMEOUT; // not sure whether we should always follow this
  // int uart_timeout = 100000;
	
  while( n < length )
  {
    TRANSPORT_VERIFY_OPEN;

    if ( adispatch_buff < 0 )
      c = platform_uart_recv( tpt->fd, tpt->tmr_id, uart_timeout );
    else
    {
      c = adispatch_buff;
      adispatch_buff = -1;
    }

    if( c < 0 )
    {
      // uart_timeout = 1000000;  // Reset and use timeout of 1s
      e.errnum = ERR_NODATA;
      e.type = nonfatal;
      Throw( e );
    }
    else
    {
      buffer[ n ] = ( u8 )c;
      n++;
    }
    // After getting one char of a read remainder
    //  should follow within a timeout of 0.1 sec
    uart_timeout = 100000;
  }
}

void transport_write_buffer( Transport *tpt, const u8 *buffer, int length )
{
  int i;
  struct exception e;
  TRANSPORT_VERIFY_OPEN;
	
  for( i = 0; i < length; i ++ )
    platform_uart_send( tpt->fd, buffer[ i ] );
}

// Check if data is available on connection without reading:
//     - 1 = data available, 0 = no data available
int transport_readable( Transport *tpt )
{
  int c;

  if ( adispatch_buff >= 0 ) // if we have a char already
    return 1;
  else // check if a char is ready to be read
  {
    c = transport_get_char( tpt );
    if( c > 0)
    {
      adispatch_buff = c;
      return 1;
    }
  }
  return 0;
}

// Check if transport is open:
//    - 1 = connection open, 0 = connection closed
int transport_is_open( Transport *tpt )
{
  return ( tpt->fd != INVALID_TRANSPORT );
}

// Shut down connection
void transport_close( Transport *tpt )
{
  tpt->fd = INVALID_TRANSPORT;
}

#endif
