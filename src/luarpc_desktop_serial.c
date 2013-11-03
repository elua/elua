#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#ifdef WIN32_BUILD
#include <malloc.h>
#else
#include <alloca.h>
#endif

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include "luarpc_rpc.h"
#include "rpc_serial.h"

#ifndef LUA_CROSS_COMPILER
#include "platform_conf.h"
#endif

#ifdef LUA_RPC
#include "desktop_conf.h"
#endif

void transport_open( Transport *tpt, const char *path );

#ifdef LUARPC_ENABLE_SERIAL

// Setup Transport 
void transport_init (Transport *tpt)
{
  tpt->fd = INVALID_TRANSPORT;
}

void transport_open( Transport *tpt, const char *path )
{
  struct exception e;
  
  tpt->fd = ser_open( path );

  if( tpt->fd == INVALID_TRANSPORT)
  {
    e.errnum = transport_errno;
    e.type = fatal;
    Throw( e );
  }
  
  ser_setup( tpt->fd, 115200, SER_DATABITS_8, SER_PARITY_NONE, SER_STOPBITS_1 );
  ser_set_timeout_ms( tpt->fd, 10000 );
}

// Open Listener / Server 
void transport_open_listener(lua_State *L, ServerHandle *handle)
{
  check_num_args (L,2); // 1st arg is path, 2nd is handle
  if (!lua_isstring (L,1))
    luaL_error(L,"first argument must be serial serial port");

  transport_open( &handle->ltpt, lua_tostring (L,1) );
    
  while( transport_readable( &handle->ltpt ) == 0 ); // wait for incoming data
}

// Open Connection / Client
int transport_open_connection(lua_State *L, Handle *handle)
{ 
  check_num_args (L,2); // 1st arg is path, 2nd is handle
  if (!lua_isstring (L,1))
    luaL_error(L,"first argument must be serial serial port");

  transport_open( &handle->tpt, lua_tostring (L,1) );
  
  return 1;
}

// Accept Connection
void transport_accept (Transport *tpt, Transport *atpt)
{
  struct exception e;
  TRANSPORT_VERIFY_OPEN;
  while( transport_readable( tpt ) == 0 ); // wait for incoming data
  
  atpt->fd = tpt->fd;
}


// Read & Write to Transport
void transport_read_buffer (Transport *tpt, u8 *buffer, int length)
{
  int n;
  struct exception e;
  TRANSPORT_VERIFY_OPEN;
  
  while( length > 0 )
  {
    TRANSPORT_VERIFY_OPEN;

    n = ser_read( tpt->fd, buffer, length );
    
    // error handling
    if( n == 0 )
    {
      e.errnum = ERR_NODATA;
      e.type = nonfatal;
      Throw( e );
    }
    
    if( n < 0 )
    {
      e.errnum = transport_errno;
      e.type = fatal;
      Throw( e );
    }
   
    buffer += n;
    length -= n;
  }
}

void transport_write_buffer( Transport *tpt, const u8 *buffer, int length )
{
  int n;
  struct exception e;
  TRANSPORT_VERIFY_OPEN;

  n = ser_write( tpt->fd, buffer, length );

  if ( n != length )
  {
    e.errnum = transport_errno;
    e.type = fatal;
    Throw( e );
  }
}

// Check if data is available on connection without reading:
//    - 1 = data available, 0 = no data available
int transport_readable (Transport *tpt)
{
  struct exception e;
  int ret;

  if (tpt->fd == INVALID_TRANSPORT)
    return 0;
  
  ret = ser_readable( tpt->fd );
  
  if ( ret < 0 )
  {
    e.errnum = transport_errno;
    e.type = fatal;
    Throw( e );
  }
  
  return ( ret > 0 );
}

// Check if transport is open:
//    1 = connection open, 0 = connection closed
int transport_is_open (Transport *tpt)
{
  return (tpt->fd != INVALID_TRANSPORT);
}

// Shut down connection
void transport_close (Transport *tpt)
{
  if (tpt->fd != INVALID_TRANSPORT)
  {
    ser_close( tpt->fd );
    tpt->fd = INVALID_TRANSPORT;
  }
}

#endif // LUARPC_ENABLE_SERIAL
