// Module for interfacing with network functions (elua_net.h)

//#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "auxmods.h"
#include "elua_net.h"
#include "common.h"
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include "lrotable.h"

#include "platform_conf.h"
#ifdef BUILD_UIP


/*
 * TH: Added listen/unlisten functions
 */

//Lua: err=listen(port)
static int net_listen( lua_State *L )
{
  u16 port = ( u16 )luaL_checkinteger( L, 1 );
  lua_pushinteger (L,( elua_listen( port,TRUE )==0) ?ELUA_NET_ERR_OK:ELUA_NET_ERR_LIMIT_EXCEEDED );
  return 1;
}


//Lua: unlisten(port)
//  will always return ELUA_NET_ERR_OK because there is no check
// if the was active listening in the port.
static int net_unlisten(lua_State *L)
{
  u16 port = ( u16 )luaL_checkinteger( L, 1 );
  elua_listen( port,FALSE );
  lua_pushinteger(L,ELUA_NET_ERR_OK);
  return 1;
}


// Lua: sock, remoteip, err = accept( port, [timer_id, timeout] )
static int net_accept( lua_State *L )
{
  u16 port = ( u16 )luaL_checkinteger( L, 1 );

  // to be compatible with orginal net we just start listening to the port
  // here. The recommended usage is to use listen/unlisten to specifiy
  // listening and use the accept call to take incomming connections the port
  int lres=elua_listen(port,TRUE);
  if ( lres!=0 )
  { // check if listen was succesfull
    // Remark: With uIP listen is always successfull, even if there are
    // no free listen slots. This is a limitation of the library
    // Nevertheless I kept the error handler for completness.
    // if not return with error
    lua_pushinteger(L,-1); // sock
    lua_pushinteger(L,0); // rem ip
    lua_pushinteger(L,ELUA_NET_ERR_LIMIT_EXCEEDED);
    return 3;
  }


  unsigned timer_id = PLATFORM_TIMER_SYS_ID;
  timer_data_type timeout = PLATFORM_TIMER_INF_TIMEOUT;
  elua_net_ip remip;
  int sock;

  cmn_get_timeout_data( L, 2, &timer_id, &timeout );

  lua_pushinteger( L,sock = elua_accept( port, timer_id, timeout, &remip ) );
  lua_pushinteger( L, ( sock>=0 ) ? remip.ipaddr:0 );
  lua_pushinteger( L, ( sock>=0 )? elua_net_get_last_err( sock ):ELUA_NET_ERR_WAIT_TIMEDOUT );
  return 3;
}

// Lua: sock = socket( type )
static int net_socket( lua_State *L )
{
  int type = ( int )luaL_checkinteger( L, 1 );

  lua_pushinteger( L, elua_net_socket( type ) );
  return 1;
}

// Lua: res = close( socket )
static int net_close( lua_State* L )
{
  int sock = ( int )luaL_checkinteger( L, 1 );

  lua_pushinteger( L, elua_net_close( sock ) );
  return 1;
}

// Lua: res, err = send( sock, str )
static int net_send( lua_State* L )
{
  int sock = ( int )luaL_checkinteger( L, 1 );
  const char *buf;
  size_t len;

  luaL_checktype( L, 2, LUA_TSTRING );
  buf = lua_tolstring( L, 2, &len );
  lua_pushinteger( L, elua_net_send( sock, buf, len ) );
  lua_pushinteger( L, elua_net_get_last_err( sock ) );
  return 2;
}

// Lua: err = connect( sock, iptype, port )
// "iptype" is actually an int returned by "net.packip"
static int net_connect( lua_State *L )
{
  elua_net_ip ip;
  int sock = ( int )luaL_checkinteger( L, 1 );
  u16 port = ( int )luaL_checkinteger( L, 3 );

  ip.ipaddr = ( u32 )luaL_checkinteger( L, 2 );
  elua_net_connect( sock, ip, port );
  lua_pushinteger( L, elua_net_get_last_err( sock ) );
  return 1;
}

// Lua: data = packip( ip0, ip1, ip2, ip3 ), or
// Lua: data = packip( "ip" )
// Returns an internal representation for the given IP address
static int net_packip( lua_State *L )
{
  elua_net_ip ip;
  unsigned i, temp;

  if( lua_isnumber( L, 1 ) )
    for( i = 0; i < 4; i ++ )
    {
      temp = luaL_checkinteger( L, i + 1 );
      if( temp < 0 || temp > 255 )
        return luaL_error( L, "invalid IP adddress" );
      ip.ipbytes[ i ] = temp;
    }
  else
  {
    const char* pip = luaL_checkstring( L, 1 );
    unsigned len, temp[ 4 ];

    if( sscanf( pip, "%u.%u.%u.%u%n", temp, temp + 1, temp + 2, temp + 3, &len ) != 4 || len != strlen( pip ) )
      return luaL_error( L, "invalid IP adddress" );
    for( i = 0; i < 4; i ++ )
    {
      if( temp[ i ] < 0 || temp[ i ] > 255 )
        return luaL_error( L, "invalid IP address" );
      ip.ipbytes[ i ] = ( u8 )temp[ i ];
    }
  }
  lua_pushinteger( L, ip.ipaddr );
  return 1;
}

// Lua: ip0, ip1, ip2, ip3 = unpackip( iptype, "*n" ), or
//               string_ip = unpackip( iptype, "*s" )
static int net_unpackip( lua_State *L )
{
  elua_net_ip ip;
  unsigned i;
  const char* fmt;

  ip.ipaddr = ( u32 )luaL_checkinteger( L, 1 );
  fmt = luaL_checkstring( L, 2 );
  if( !strcmp( fmt, "*n" ) )
  {
    for( i = 0; i < 4; i ++ )
      lua_pushinteger( L, ip.ipbytes[ i ] );
    return 4;
  }
  else if( !strcmp( fmt, "*s" ) )
  {
    lua_pushfstring( L, "%d.%d.%d.%d", ( int )ip.ipbytes[ 0 ], ( int )ip.ipbytes[ 1 ],
                     ( int )ip.ipbytes[ 2 ], ( int )ip.ipbytes[ 3 ] );
    return 1;
  }
  else
    return luaL_error( L, "invalid format" );
}

// Lua: res, err = recv( sock, maxsize, [timer_id, timeout] ) or
//      res, err = recv( sock, "*l", [timer_id, timeout] )
static int net_recv( lua_State *L )
{
  int sock = ( int )luaL_checkinteger( L, 1 );
  elua_net_size maxsize;
  s16 lastchar = ELUA_NET_NO_LASTCHAR;
  unsigned timer_id = PLATFORM_TIMER_SYS_ID;
  timer_data_type timeout = PLATFORM_TIMER_INF_TIMEOUT;
  luaL_Buffer net_recv_buff;

  if( lua_isnumber( L, 2 ) ) // invocation with maxsize
    maxsize = ( elua_net_size )luaL_checkinteger( L, 2 );
  else // invocation with line mode
  {
    if( strcmp( luaL_checkstring( L, 2 ), "*l" ) )
      return luaL_error( L, "invalid second argument to recv" );
    lastchar = '\n';
    maxsize = BUFSIZ;
  }
  cmn_get_timeout_data( L, 3, &timer_id, &timeout );
  // Initialize buffer
  luaL_buffinit( L, &net_recv_buff );
  elua_net_recvbuf( sock, &net_recv_buff, maxsize, lastchar, timer_id, timeout );
  luaL_pushresult( &net_recv_buff );
  lua_pushinteger( L, elua_net_get_last_err( sock ) );
  return 2;
}

// Lua: iptype = lookup( "name" )
static int net_lookup( lua_State* L )
{
  const char* name = luaL_checkstring( L, 1 );
  elua_net_ip res;

  res = elua_net_lookup( name );
  lua_pushinteger( L, res.ipaddr );
  return 1;
}

// Module function map
#define MIN_OPT_LEVEL 2
#include "lrodefs.h"
const LUA_REG_TYPE net_map[] =
{
  { LSTRKEY( "accept" ), LFUNCVAL( net_accept ) },
  { LSTRKEY( "packip" ), LFUNCVAL( net_packip ) },
  { LSTRKEY( "unpackip" ), LFUNCVAL( net_unpackip ) },
  { LSTRKEY( "connect" ), LFUNCVAL( net_connect ) },
  { LSTRKEY( "socket" ), LFUNCVAL( net_socket ) },
  { LSTRKEY( "close" ), LFUNCVAL( net_close ) },
  { LSTRKEY( "send" ), LFUNCVAL( net_send ) },
  { LSTRKEY( "recv" ), LFUNCVAL( net_recv ) },
  { LSTRKEY( "lookup" ), LFUNCVAL( net_lookup ) },
  { LSTRKEY( "listen" ), LFUNCVAL( net_listen ) }, // TH
  { LSTRKEY( "unlisten" ), LFUNCVAL( net_unlisten ) }, // TH
#if LUA_OPTIMIZE_MEMORY > 0
  { LSTRKEY( "SOCK_STREAM" ), LNUMVAL( ELUA_NET_SOCK_STREAM ) },
  { LSTRKEY( "SOCK_DGRAM" ), LNUMVAL( ELUA_NET_SOCK_DGRAM ) },
  { LSTRKEY( "ERR_OK" ), LNUMVAL( ELUA_NET_ERR_OK ) },
  { LSTRKEY( "ERR_TIMEOUT" ), LNUMVAL( ELUA_NET_ERR_TIMEDOUT ) },
  { LSTRKEY( "ERR_CLOSED" ), LNUMVAL( ELUA_NET_ERR_CLOSED ) },
  { LSTRKEY( "ERR_ABORTED" ), LNUMVAL( ELUA_NET_ERR_ABORTED ) },
  { LSTRKEY( "ERR_OVERFLOW" ), LNUMVAL( ELUA_NET_ERR_OVERFLOW ) },
  { LSTRKEY( "ERR_LIMIT_EXCEEDED" ), LNUMVAL( ELUA_NET_ERR_LIMIT_EXCEEDED ) }, //TH
  { LSTRKEY( "ERR_WAIT_TIMEDOUT" ), LNUMVAL( ELUA_NET_ERR_WAIT_TIMEDOUT ) }, //TH

  { LSTRKEY( "NO_TIMEOUT" ), LNUMVAL( 0 ) },
  { LSTRKEY( "INF_TIMEOUT" ), LNUMVAL( PLATFORM_TIMER_INF_TIMEOUT ) },
#endif
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_net( lua_State *L )
{
#if LUA_OPTIMIZE_MEMORY > 0
  return 0;
#else // #if LUA_OPTIMIZE_MEMORY > 0
  luaL_register( L, AUXLIB_NET, net_map );

  // Module constants
  MOD_REG_NUMBER( L, "SOCK_STREAM", ELUA_NET_SOCK_STREAM );
  MOD_REG_NUMBER( L, "SOCK_DGRAM", ELUA_NET_SOCK_DGRAM );
  MOD_REG_NUMBER( L, "ERR_OK", ELUA_NET_ERR_OK );
  MOD_REG_NUMBER( L, "ERR_TIMEDOUT", ELUA_NET_ERR_TIMEDOUT );
  MOD_REG_NUMBER( L, "ERR_CLOSED", ELUA_NET_ERR_CLOSED );
  MOD_REG_NUMBER( L, "ERR_ABORTED", ELUA_NET_ERR_ABORTED );
  MOD_REG_NUMBER( L, "ERR_OVERFLOW", ELUA_NET_ERR_OVERFLOW );
  MOD_REG_NUMBER( L, "NO_TIMEOUT", 0 );
  MOD_REG_NUMBER( L, "INF_TIMEOUT", PLATFORM_TIMER_INF_TIMEOUT );

  return 1;
#endif // #if LUA_OPTIMIZE_MEMORY > 0
}

#else // #ifdef BUILD_UIP

LUALIB_API int luaopen_net( lua_State *L )
{
  return 0;
}

#endif // #ifdef BUILD_UIP

