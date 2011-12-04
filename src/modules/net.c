// Module for interfacing with network functions (elua_net.h)

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "auxmods.h"
#include "elua_net.h"
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include "lrotable.h"
#include "common.h"

#include "platform_conf.h"
#ifdef BUILD_UIP

#define lua_puship( L, ip )   lua_pushnumber( L, ( lua_Number )ip )

#define NET_META_NAME           "eLua.net"
#define sock_check( L )         ( sock_t* )luaL_checkudata( L, 1, NET_META_NAME )

typedef struct
{
  int sock;
} sock_t;

// Lua: sock, remoteip, err = accept( port, [bufsize], [timeout], [timer_id] )
static int net_accept( lua_State *L )
{
  u16 port = ( u16 )luaL_checkinteger( L, 1 );
  unsigned bufsize = ( unsigned )luaL_optinteger( L, 2, 0 );
  unsigned timer_id = PLATFORM_TIMER_SYS_ID;
  timer_data_type timeout = PLATFORM_TIMER_INF_TIMEOUT;
  elua_net_ip remip;
  int res;
  sock_t *s;

  cmn_get_timeout_data( L, 3, &timeout, &timer_id );
  if( ( res = elua_net_accept( port, bufsize, timer_id, timeout, &remip ) ) != -1 )
  {
    s = ( sock_t* )lua_newuserdata( L, sizeof( sock_t ) );
    s->sock = res;
    luaL_getmetatable( L, NET_META_NAME );
    lua_setmetatable( L, -2 );
    lua_puship( L, remip.ipaddr );
    lua_pushinteger( L, elua_net_get_last_err( res ) );
    return 3;
  }
  else
    return 0;
}

// Lua: sock = socket( type )
static int net_socket( lua_State *L )
{
  int type = ( int )luaL_checkinteger( L, 1 );
  int res;
  sock_t *s;
  
  res = elua_net_socket( type );
  if( res != ELUA_NET_INVALID_SOCKET )
  {
    s = ( sock_t* )lua_newuserdata( L, sizeof( sock_t ) );
    s->sock = res;
    luaL_getmetatable( L, NET_META_NAME );
    lua_setmetatable( L, -2 );
    return 1;
  }
  else
    return 0;
}

// Lua: set_buffer( sock, size )
// Use '0' to disable buffering
static int net_set_buffer( lua_State *L )
{
  sock_t *s = sock_check( L );
  unsigned bufsize = ( unsigned )luaL_checkinteger( L, 2 );
  
  if( elua_net_set_buffer( s->sock, bufsize ) == 0 )
    return luaL_error( L, "unable to set buffer on socket %d", s->sock );
  return 0;
}

// Lua: res = set_split( sock, [split_char])
// Defaults to 'no split' if not specified
static int net_set_split( lua_State *L )
{
  sock_t *s = sock_check( L );
  int schar = luaL_optinteger( L, 2, ELUA_NET_NO_SPLIT );

  lua_pushboolean( L, elua_net_set_split( s->sock, schar ) );
  return 1;
}

// Lua: res = close( socket )
static int net_close( lua_State* L )
{
  sock_t *s = sock_check( L );
  int res;

  if( ( res = elua_net_close( s->sock ) ) != 0 )
    s->sock = -1;
  lua_pushinteger( L, res );
  return 1;
}

// Lua: res, err = send( sock, str )
static int net_send( lua_State* L )
{
  sock_t *s = sock_check( L );
  const char *buf;
  size_t len;
    
  luaL_checktype( L, 2, LUA_TSTRING );
  buf = lua_tolstring( L, 2, &len );
  lua_pushinteger( L, elua_net_send( s->sock, buf, len ) );
  lua_pushinteger( L, elua_net_get_last_err( s->sock ) );
  return 2;  
}

// Lua: err = connect( sock, iptype, port, [timeout], [timer_id] )
// "iptype" is actually an int returned by "net.packip"
static int net_connect( lua_State *L )
{
  elua_net_ip ip;
  sock_t *s = sock_check( L );
  u16 port = ( u16 )luaL_checkinteger( L, 3 );
  unsigned timer_id = PLATFORM_TIMER_SYS_ID;
  timer_data_type timeout = PLATFORM_TIMER_INF_TIMEOUT;
  
  cmn_get_timeout_data( L, 4, &timeout, &timer_id );  
  luaL_checkinteger( L, 2 );
  ip.ipaddr = ( u32 )luaL_checknumber( L, 2 );
  elua_net_connect( s->sock, ip, port, timer_id, timeout );
  lua_pushinteger( L, elua_net_get_last_err( s->sock ) );
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
  lua_puship( L, ip.ipaddr );
  return 1;
}

// Lua: ip0, ip1, ip2, ip3 = unpackip( iptype, "*n" ), or
//               string_ip = unpackip( iptype, "*s" )
static int net_unpackip( lua_State *L )
{
  elua_net_ip ip;
  unsigned i;  
  const char* fmt;
  
  luaL_checkinteger( L, 1 );
  ip.ipaddr = ( u32 )luaL_checknumber( L, 1 );
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

// Lua: res, err = recv( sock, maxsize, [timeout], [timer_id] )
static int net_recv( lua_State *L )
{
  sock_t *s = sock_check( L );
  elua_net_size maxsize = ( elua_net_size )luaL_checkinteger( L, 2 );
  unsigned timer_id = PLATFORM_TIMER_SYS_ID;
  timer_data_type timeout = PLATFORM_TIMER_INF_TIMEOUT;
  luaL_Buffer net_recv_buff;

  cmn_get_timeout_data( L, 3, &timeout, &timer_id );
  // Initialize buffer
  luaL_buffinit( L, &net_recv_buff );
  elua_net_recvbuf( s->sock, &net_recv_buff, maxsize, timer_id, timeout );
  luaL_pushresult( &net_recv_buff );
  lua_pushinteger( L, elua_net_get_last_err( s->sock ) );
  return 2;
}

// Lua: iptype = lookup( "name" )
static int net_lookup( lua_State* L )
{
  const char* name = luaL_checkstring( L, 1 );
  elua_net_ip res;
  
  res = elua_net_lookup( name );
  lua_puship( L, res.ipaddr );
  return 1;
}

// Lua: ip, netmask, dns, gw = netcfg()
// Returns 'nil' if the network isn't initialized yet
static int net_netcfg( lua_State *L )
{
  elua_net_ip ip;

  ip = elua_net_get_config( ELUA_NET_CFG_IP );
  if( ip.ipaddr == 0 )
    return 0;
  lua_puship( L, ip.ipaddr );
  ip = elua_net_get_config( ELUA_NET_CFG_NETMASK );
  lua_puship( L, ip.ipaddr );
  ip = elua_net_get_config( ELUA_NET_CFG_DNS );
  lua_puship( L, ip.ipaddr );
  ip = elua_net_get_config( ELUA_NET_CFG_GW );
  lua_puship( L, ip.ipaddr );
  return 4;
}

// Lua: res, err = sendto( socket, data, remoteip, remoteport )
static int net_sendto( lua_State *L )
{
  sock_t *s = sock_check( L );
  u16 port = ( u16 )luaL_checkinteger( L, 4 );
  elua_net_ip ip;
  const char *buf;
  size_t len;
    
  luaL_checktype( L, 2, LUA_TSTRING );
  buf = lua_tolstring( L, 2, &len );
  luaL_checkinteger( L, 3 );
  ip.ipaddr = ( u32 )luaL_checknumber( L, 3 );
  lua_pushinteger( L, elua_net_sendto( s->sock, buf, len, ip, port ) );
  lua_pushinteger( L, elua_net_get_last_err( s->sock ) );
  return 2; 
}

// Lua: res, err, remoteip, remoteport = recvfrom( sock, maxsize, [timeout], [timer_id] )
static int net_recvfrom( lua_State *L )
{
  sock_t *s = sock_check( L );
  elua_net_size maxsize = ( elua_net_size )luaL_checkinteger( L, 2 );
  unsigned timer_id = PLATFORM_TIMER_SYS_ID;
  timer_data_type timeout = PLATFORM_TIMER_INF_TIMEOUT;
  luaL_Buffer net_recv_buff;
  elua_net_ip remoteip;
  u16 remoteport;

  cmn_get_timeout_data( L, 3, &timeout, &timer_id );
  // Initialize buffer
  luaL_buffinit( L, &net_recv_buff );
  elua_net_recvfrombuf( s->sock, &net_recv_buff, maxsize, &remoteip, &remoteport, timer_id, timeout );
  luaL_pushresult( &net_recv_buff );
  lua_pushinteger( L, elua_net_get_last_err( s->sock ) );
  lua_puship( L, remoteip.ipaddr );
  lua_pushinteger( L, remoteport );
  return 4;
}

// Lua: res = net.expect( sock, s, [timeout], [timer_id] )
static int net_expect( lua_State *L )
{
  const char *se;
  sock_t *s = sock_check( L );
  size_t len;
  timer_data_type timeout = PLATFORM_TIMER_INF_TIMEOUT;
  unsigned timer_id = PLATFORM_TIMER_SYS_ID;

  cmn_get_timeout_data( L, 3, &timeout, &timer_id );
  se = luaL_checklstring( L, 2, &len );
  lua_pushboolean( L, elua_net_expect( s->sock, ( const u8* )se, len, timer_id, timeout ) );
  return 1;
}

// Lua: buf = net.readto( sock, s, [timeout], [timer_id] )
static int net_readto( lua_State *L )
{
  const char *se;
  size_t len;
  timer_data_type timeout = PLATFORM_TIMER_INF_TIMEOUT;
  unsigned timer_id = PLATFORM_TIMER_SYS_ID;
  sock_t *s = sock_check( L );
  luaL_Buffer b;

  se = luaL_checklstring( L, 2, &len );
  cmn_get_timeout_data( L, 3, &timeout, &timer_id );
  luaL_buffinit( L, &b );
  elua_net_readto( s->sock, &b, ( const u8* )se, len, timer_id, timeout );
  luaL_pushresult( &b );
  return 1;
}

// Lua: buf = net.read_tags( sock, tag1, tag2, [timeout], [timer_id] )
static int net_read_tags( lua_State *L )
{
  const char *tag1, *tag2;
  size_t len1, len2;
  timer_data_type timeout = PLATFORM_TIMER_INF_TIMEOUT;
  unsigned timer_id = PLATFORM_TIMER_SYS_ID;
  luaL_Buffer b;
  sock_t *s = sock_check( L );

  tag1 = luaL_checklstring( L, 2, &len1 );
  tag2 = luaL_checklstring( L, 3, &len2 );
  cmn_get_timeout_data( L, 4, &timeout, &timer_id );
  luaL_buffinit( L, &b );
  if( elua_net_expect( s->sock, ( const u8* )tag1, len1, timer_id, timeout ) )
    elua_net_readto( s->sock, &b, ( const u8* )tag2, len2, timer_id, timeout );
  luaL_pushresult( &b );
  return 1; 
}

// Garbage collection function for sockets
static int socket_gc( lua_State *L )
{
  sock_t *s = sock_check( L );
  
  if( s && s->sock != -1 )
  {
    elua_net_close( s->sock );
    s->sock = -1;
  }
  return 0; 
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
  { LSTRKEY( "set_buffer" ), LFUNCVAL( net_set_buffer ) },
  { LSTRKEY( "set_split" ), LFUNCVAL( net_set_split ) },
  { LSTRKEY( "close" ), LFUNCVAL( net_close ) },
  { LSTRKEY( "send" ), LFUNCVAL( net_send ) },
  { LSTRKEY( "recv" ), LFUNCVAL( net_recv ) },
  { LSTRKEY( "lookup" ), LFUNCVAL( net_lookup ) },
  { LSTRKEY( "netcfg" ), LFUNCVAL( net_netcfg ) },
  { LSTRKEY( "sendto" ), LFUNCVAL( net_sendto ) },
  { LSTRKEY( "recvfrom" ), LFUNCVAL( net_recvfrom ) },
  { LSTRKEY( "expect" ), LFUNCVAL( net_expect ) },
  { LSTRKEY( "readto" ), LFUNCVAL( net_readto ) },
  { LSTRKEY( "read_tags" ), LFUNCVAL( net_read_tags ) },
#if LUA_OPTIMIZE_MEMORY > 0
  { LSTRKEY( "SOCK_STREAM" ), LNUMVAL( ELUA_NET_SOCK_STREAM ) },
  { LSTRKEY( "SOCK_DGRAM" ), LNUMVAL( ELUA_NET_SOCK_DGRAM ) },
  { LSTRKEY( "ERR_OK" ), LNUMVAL( ELUA_NET_ERR_OK ) },
  { LSTRKEY( "ERR_TIMEOUT" ), LNUMVAL( ELUA_NET_ERR_TIMEDOUT ) },
  { LSTRKEY( "ERR_CLOSED" ), LNUMVAL( ELUA_NET_ERR_CLOSED ) },
  { LSTRKEY( "ERR_ABORTED" ), LNUMVAL( ELUA_NET_ERR_ABORTED ) },
  { LSTRKEY( "ERR_OVERFLOW" ), LNUMVAL( ELUA_NET_ERR_OVERFLOW ) },
  { LSTRKEY( "SYS_TIMER" ), LNUMVAL( PLATFORM_TIMER_SYS_ID ) },
  { LSTRKEY( "NO_TIMEOUT" ), LNUMVAL( 0 ) },
  { LSTRKEY( "INVALID_SOCKET" ), LNUMVAL( ELUA_NET_INVALID_SOCKET ) },
  { LSTRKEY( "INF_TIMEOUT" ), LNUMVAL( PLATFORM_TIMER_INF_TIMEOUT ) },
  { LSTRKEY( "NO_SPLIT" ), LNUMVAL( ELUA_NET_NO_SPLIT ) },
#endif
  { LNILKEY, LNILVAL }
};

static const LUA_REG_TYPE socket_mt_map[] = 
{
  { LSTRKEY( "__gc" ), LFUNCVAL( socket_gc ) },
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_net( lua_State *L )
{
#if LUA_OPTIMIZE_MEMORY > 0
  // Create default font metatable
  luaL_rometatable( L, NET_META_NAME, ( void* )socket_mt_map );
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
  MOD_REG_NUMBER( L, "SYS_TIMER", PLATFORM_TIMER_SYS_ID );
  MOD_REG_NUMBER( L, "NO_TIMEOUT", 0 );
  MOD_REG_NUMBER( L, "INVALID_SOCKET", ELUA_NET_INVALID_SOCKET );
  MOD_REG_NUMBER( L, "INF_TIMEOUT", PLATFORM_TIMER_INF_TIMEOUT );
  MOD_REG_NUMEBR( L, "NO_SPLIT", ELUA_NET_NO_SPLIT );
  
  return 1;
#endif // #if LUA_OPTIMIZE_MEMORY > 0  
}

#else // #ifdef BUILD_UIP

LUALIB_API int luaopen_net( lua_State *L )
{
  return 0;
}

#endif // #ifdef BUILD_UIP

