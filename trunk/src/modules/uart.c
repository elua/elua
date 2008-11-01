// Module for interfacing with UART

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "auxmods.h"

// Lua: actualbaud = setup( id, baud, databits, parity, stopbits )
static int uart_setup( lua_State* L )
{
  unsigned id, databits, parity, stopbits;
  u32 baud, res;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( uart, id );
  baud = luaL_checkinteger( L, 2 );
  databits = luaL_checkinteger( L, 3 );
  parity = luaL_checkinteger( L, 4 );
  stopbits = luaL_checkinteger( L, 5 );
  res = platform_uart_setup( id, baud, databits, parity, stopbits );
  lua_pushinteger( L, res );
  return 1;
}

// Lua: send( id, out1, out2, ... )
static int uart_send( lua_State* L )
{
  u8 value;
  int total = lua_gettop( L ), i, id;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( uart, id );
  for( i = 2; i <= total; i ++ )
  {
    value = luaL_checkinteger( L, i );  
    platform_uart_send( id, value );
  }
  return 0;
}

// Lua: sendstr( id, string1, string2, ... )
static int uart_sendstr( lua_State* L )
{
  int id;
  const char* buf;
  size_t len, i;
  int total = lua_gettop( L ), s;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( uart, id );
  for( s = 2; s <= total; s ++ )
  {
    luaL_checktype( L, s, LUA_TSTRING );
    buf = lua_tolstring( L, s, &len );
    for( i = 0; i < len; i ++ )
      platform_uart_send( id, buf[ i ] );
  }
  return 0;
}

// Lua: data = recv( id, timer_id, timeout )
static int uart_recv( lua_State* L )
{
  int id, timer_id, timeout, res;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( uart, id );
  timer_id = luaL_checkinteger( L, 2 );
  timeout = luaL_checkinteger( L, 3 );
  res = platform_uart_recv( id, timer_id, timeout );
  lua_pushinteger( L, res );
  return 1;  
}

// Module function map
static const luaL_reg uart_map[] = 
{
  { "setup",  uart_setup },
  { "send", uart_send },
  { "recv", uart_recv },
  { "sendstr", uart_sendstr },
  { NULL, NULL }
};

LUALIB_API int luaopen_uart( lua_State *L )
{
  luaL_register( L, AUXLIB_UART, uart_map );
  
  // Add the stop bits and parity constants (for uart.setup)
  lua_pushnumber( L, PLATFORM_UART_PARITY_EVEN );
  lua_setfield( L, -2, "PAR_EVEN" );
  lua_pushnumber( L, PLATFORM_UART_PARITY_ODD );
  lua_setfield( L, -2, "PAR_ODD" );  
  lua_pushnumber( L, PLATFORM_UART_PARITY_NONE );
  lua_setfield( L, -2, "PAR_NONE" );  
  lua_pushnumber( L, PLATFORM_UART_STOPBITS_1 );
  lua_setfield( L, -2, "STOP_1" );
  lua_pushnumber( L, PLATFORM_UART_STOPBITS_1_5 );
  lua_setfield( L, -2, "STOP_1_5" );  
  lua_pushnumber( L, PLATFORM_UART_STOPBITS_2 );
  lua_setfield( L, -2, "STOP_2" );    
  
  // Add the "none" and "infinite" constant used in recv()
  lua_pushnumber( L, 0 );
  lua_setfield( L, -2, "NO_TIMEOUT" );
  lua_pushnumber( L, PLATFORM_UART_INFINITE_TIMEOUT );
  lua_setfield( L, -2, "INF_TIMEOUT" );
  
  return 1;
}
