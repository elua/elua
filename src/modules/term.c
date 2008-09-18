// Module for interfacing with terminal functions

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "auxmods.h"
#include "term.h"
#include "build.h"

// Lua: clrscr()
static int luaterm_clrscr( lua_State* L )
{
  term_clrscr();
  return 0;
}

// Lua: clreol()
static int luaterm_clreol( lua_State* L )
{
  term_clreol();
  return 0;
}

// Lua: gotoxy( x, y )
static int luaterm_gotoxy( lua_State* L )
{
  unsigned x, y;
  
  x = ( unsigned )luaL_checkinteger( L, 1 );
  y = ( unsigned )luaL_checkinteger( L, 2 );
  term_gotoxy( x, y );
  return 0;
}

// Lua: up( lines )
static int luaterm_up( lua_State* L )
{
  unsigned delta;
  
  delta = ( unsigned )luaL_checkinteger( L, 1 );
  term_up( delta );
  return 0;
}

// Lua: down( lines )
static int luaterm_down( lua_State* L )
{
  unsigned delta;
  
  delta = ( unsigned )luaL_checkinteger( L, 1 );
  term_down( delta );
  return 0;
}

// Lua: left( cols )
static int luaterm_left( lua_State* L )
{
  unsigned delta;
  
  delta = ( unsigned )luaL_checkinteger( L, 1 );
  term_left( delta );
  return 0;
}

// Lua: right( cols )
static int luaterm_right( lua_State* L )
{
  unsigned delta;
  
  delta = ( unsigned )luaL_checkinteger( L, 1 );
  term_right( delta );
  return 0;
}

// Lua: lines = lines()
static int luaterm_lines( lua_State* L )
{
  lua_pushinteger( L, term_get_lines() );
  return 1;
}

// Lua: columns = cols()
static int luaterm_cols( lua_State* L )
{
  lua_pushinteger( L, term_get_cols() );
  return 1;
}

// Lua: put( c1, c2, ... )
static int luaterm_put( lua_State* L )
{
  int total = lua_gettop( L ), i;
  u8 data;
  
  for( i = 1; i <= total; i ++ )
  {
    data = ( u8 )luaL_checkinteger( L, 1 );
    term_putch( data );
  }
  return 0;
}

// Lua: putxy( x, y, c1, c2, ... )
static int luaterm_putxy( lua_State* L )
{
  int total = lua_gettop( L ), i;
  unsigned x, y;
  u8 data;
  
  x = ( unsigned )luaL_checkinteger( L, 1 );
  y = ( unsigned )luaL_checkinteger( L, 2 );
  term_gotoxy( x, y );
  for( i = 3; i <= total; i ++ )
  {
    data = ( u8 )luaL_checkinteger( L, i );
    term_putch( data );
  }
  return 0;
}

// Lua: putstr( string1, string2, ... )
static int luaterm_putstr( lua_State* L )
{
  const char* buf;
  size_t len, i;
  int total = lua_gettop( L ), s;
  
  for( s = 1; s <= total; s ++ )
  {
    luaL_checktype( L, s, LUA_TSTRING );
    buf = lua_tolstring( L, s, &len );
    for( i = 0; i < len; i ++ )
      term_putch( buf[ i ] );
  }
  return 0;
}

// Lua: putstrxy( x, y, string1, string2, ... )
static int luaterm_putstrxy( lua_State* L )
{
  const char* buf;
  unsigned x, y;
  size_t len, i;
  int total = lua_gettop( L ), s;
  
  x = ( unsigned )luaL_checkinteger( L, 1 );
  y = ( unsigned )luaL_checkinteger( L, 2 );
  term_gotoxy( x, y );
  for( s = 3; s <= total; s ++ )
  {
    luaL_checktype( L, s, LUA_TSTRING );
    buf = lua_tolstring( L, s, &len );
    for( i = 0; i < len; i ++ )
      term_putch( buf[ i ] );
  }
  return 0;
}

// Lua: cursorx = cursorx()
static int luaterm_cx( lua_State* L )
{
  lua_pushinteger( L, term_get_cx() );
  return 1;
}

// Lua: cursory = cursory()
static int luaterm_cy( lua_State* L )
{
  lua_pushinteger( L, term_get_cy() );
  return 1;
}

// Lua: key = getch( mode )
static int luaterm_getch( lua_State* L )
{
  int temp;
  
  temp = luaL_checkinteger( L, 1 );
  lua_pushinteger( L, term_getch( temp ) );
  return 1;
}

// Module function map
static const luaL_reg term_map[] = 
{
  { "clrscr", luaterm_clrscr },
  { "clreol", luaterm_clreol },
  { "gotoxy", luaterm_gotoxy },
  { "up", luaterm_up },
  { "down", luaterm_down },
  { "left", luaterm_left },
  { "right", luaterm_right },
  { "lines", luaterm_lines },
  { "cols", luaterm_cols },
  { "put", luaterm_put },
  { "putstr", luaterm_putstr },
  { "putxy", luaterm_putxy },
  { "putstrxy", luaterm_putstrxy },
  { "cursorx", luaterm_cx },
  { "cursory", luaterm_cy },
  { "getch", luaterm_getch },
  { NULL, NULL }
};

// Key codes by name
#undef _D
#define _D( x ) #x
static const char* term_key_names[] = { TERM_KEYCODES };

LUALIB_API int luaopen_term( lua_State* L )
{
#ifdef BUILD_TERM
  unsigned i;
  
  // Register methods
  luaL_register( L, AUXLIB_TERM, term_map );  
  
  // Register key names
  for( i = 0; i < sizeof( term_key_names ) / sizeof( char* ); i ++ )
  {
    lua_pushnumber( L, i + TERM_FIRST_KEY );
    lua_setfield( L, -2, term_key_names[ i ] );
  }
  
  // Register the constants for "getch"
  lua_pushnumber( L, TERM_INPUT_DONT_WAIT );
  lua_setfield( L, -2, "NOWAIT" );  
  lua_pushnumber( L, TERM_INPUT_WAIT );
  lua_setfield( L, -2, "WAIT" );  
  
  return 1;
#else // #ifdef BUILD_TERM
  return 0;
#endif // #ifdef BUILD_TERM  
}
