// Module for interfacing with CPU

//#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "auxmods.h"
#include "lrotable.h"
#include <string.h>

#define _C( x ) { #x, x }
#include "platform_conf.h"

#if defined( PLATFORM_CPU_CONSTANTS_INTS ) || defined( PLATFORM_CPU_CONSTANTS_PLATFORM ) || defined( PLATFORM_CPU_CONSTANTS_CONFIGURED )
#define HAS_CPU_CONSTANTS
#endif

#if defined( HAS_CPU_CONSTANTS ) && !defined( PLATFORM_CPU_CONSTANTS_INTS )
#define PLATFORM_CPU_CONSTANTS_INTS
#endif

#if defined( HAS_CPU_CONSTANTS ) && !defined( PLATFORM_CPU_CONSTANTS_PLATFORM )
#define PLATFORM_CPU_CONSTANTS_PLATFORM
#endif

#if defined( HAS_CPU_CONSTANTS ) && !defined( PLATFORM_CPU_CONSTANTS_CONFIGURED )
#define PLATFORM_CPU_CONSTANTS_CONFIGURED
#endif


// TH: Converts a LUA Number into an unsigned 32 Bit Integer
// ignoring sign and overflow. The operation is basically
// number %  (2^32)
// See also patches to the bit module

#define TO_32BIT(L, n)                    \
  ( ( (s64)luaL_checknumber((L), (n))) & 0x0ffffffff  )



// Lua: w32( address, data )
static int cpu_w32( lua_State *L )
{
  u32 addr, data;

  luaL_checkinteger( L, 1 );
  luaL_checkinteger( L, 2 );
  addr = ( u32 )TO_32BIT( L, 1 );
  data = ( u32 )TO_32BIT( L, 2 );
  *( u32* )addr = data;
  return 0;
}

// Lua: data = r32( address )
static int cpu_r32( lua_State *L )
{
  u32 addr;

  luaL_checkinteger( L, 1 );
  addr = ( u32 )TO_32BIT( L, 1 );
  lua_pushnumber( L, ( lua_Number )( *( u32* )addr ) );
  return 1;
}

// Lua: w16( address, data )
static int cpu_w16( lua_State *L )
{
  u32 addr;
  u16 data = ( u16 )TO_32BIT( L, 2 );

  luaL_checkinteger( L, 1 );
  addr = ( u32 )TO_32BIT( L, 1 );
  *( u16* )addr = data;
  return 0;
}

// Lua: data = r16( address )
static int cpu_r16( lua_State *L )
{
  u32 addr;

  luaL_checkinteger( L, 1 );
  addr = ( u32 )TO_32BIT( L, 1 );
  lua_pushnumber( L, ( lua_Number )( *( u16* )addr ) );
  return 1;
}

// Lua: w8( address, data )
static int cpu_w8( lua_State *L )
{
  u32 addr;
  u8 data = ( u8 )TO_32BIT( L, 2 );

  luaL_checkinteger( L, 1 );
  addr = ( u32 )TO_32BIT( L, 1 );
  *( u8* )addr = data;
  return 0;
}

// Lua: data = r8( address )
static int cpu_r8( lua_State *L )
{
  u32 addr;

  luaL_checkinteger( L, 1 );
  addr = ( u32 )TO_32BIT( L, 1 );
  lua_pushnumber( L, ( lua_Number )( *( u8* )addr ) );
  return 1;
}

// Either disables or enables the given interrupt(s)
static int cpuh_int_helper( lua_State *L, int mode )
{
#ifdef BUILD_LUA_INT_HANDLERS
  unsigned i;
  elua_int_id id;
  elua_int_resnum resnum;
  int res;

  if( lua_gettop( L ) > 0 )
  {
    id = ( elua_int_id )luaL_checkinteger( L, 1 );
    if( id < ELUA_INT_FIRST_ID || id > INT_ELUA_LAST )
      return luaL_error( L, "invalid interrupt ID" );
    for( i = 2; i <= lua_gettop( L ); i ++ )
    {
      resnum = ( elua_int_resnum )luaL_checkinteger( L, i );
      res = platform_cpu_set_interrupt( id, resnum, mode );
      if( res == PLATFORM_INT_INVALID )
        return luaL_error( L, "%d is not a valid interrupt ID", ( int )id );
      else if( res == PLATFORM_INT_NOT_HANDLED )
        return luaL_error( L, "'%s' not implemented for interrupt %d with resource %d", mode == PLATFORM_CPU_ENABLE ? "sei" : "cli", ( int )id, ( int )resnum );
      else if( res == PLATFORM_INT_BAD_RESNUM )
        return luaL_error( L, "resource %d not valid for interrupt %d", ( int )resnum, ( int )id );
    }
  }
  else
#else // #ifdef BUILD_LUA_INT_HANDLERS
  if( lua_gettop( L ) > 0 )
    return luaL_error( L, "Lua interrupt support not available." );
#endif // #ifdef BUILD_LUA_INT_HANDLERS
  platform_cpu_set_global_interrupts( mode );
  return 0;

}

// Lua: cpu.cli( id, resnum1, [resnum2], ... [resnumn] )
static int cpu_cli( lua_State *L )
{
  return cpuh_int_helper( L, PLATFORM_CPU_DISABLE );
}

// Lua: cpu.sei( id, resnum1, [resnum2], ... [resnumn] )
static int cpu_sei( lua_State *L )
{
  return cpuh_int_helper( L, PLATFORM_CPU_ENABLE );
}

// Lua: frequency = clock()
static int cpu_clock( lua_State *L )
{
  lua_pushinteger( L, platform_cpu_get_frequency() );
  return 1;
}

// CPU constants list
typedef struct
{
  const char* name;
  u32 val;
} cpu_const_t;

#ifdef HAS_CPU_CONSTANTS
static const cpu_const_t cpu_constants[] =
{
  PLATFORM_CPU_CONSTANTS_INTS
  PLATFORM_CPU_CONSTANTS_PLATFORM
  PLATFORM_CPU_CONSTANTS_CONFIGURED
  { NULL, 0 }
};

static int cpu_mt_index( lua_State *L )
{
  const char *key = luaL_checkstring( L, 2 );
  unsigned i = 0;

  while( cpu_constants[ i ].name != NULL )
  {
    if( !strcmp( cpu_constants[ i ].name, key ) )
    {
      lua_pushnumber( L, cpu_constants[ i ].val );
      return 1;
    }
    i ++;
  }
  return 0;
}
#endif // #ifdef HAS_CPU_CONSTANTS

#ifdef BUILD_LUA_INT_HANDLERS

// Lua: prevhandler = cpu.set_int_handler( id, f )
static int cpu_set_int_handler( lua_State *L )
{
  int id = ( int )luaL_checkinteger( L, 1 );

  if( id < ELUA_INT_FIRST_ID || id > INT_ELUA_LAST )
    return luaL_error( L, "invalid interrupt ID" );
  if( lua_type( L, 2 ) == LUA_TFUNCTION || lua_type( L, 2 ) == LUA_TLIGHTFUNCTION || lua_type( L, 2 ) == LUA_TNIL )
  {
    if( lua_type( L, 2 ) == LUA_TNIL )
      elua_int_disable( id );
    else
      elua_int_enable( id );
    lua_settop( L, 2 ); // id f
    lua_rawgeti( L, LUA_REGISTRYINDEX, LUA_INT_HANDLER_KEY ); // id f inttable
    lua_rawgeti( L, -1, id ); // id f inttable prevf
    lua_replace( L, 1 ); // prevf f inttable
    lua_pushvalue( L, 2 ); // prevf f inttable f
    lua_rawseti( L, -2, id ); // prevf f inttable
    lua_pop( L, 2 ); // prevf
    return 1;
  }
  else
    return luaL_error( L, "invalid handler type (must be a function or nil)" );
  return 0;
}

// Lua: handler = cpu.get_int_handler( id )
static int cpu_get_int_handler( lua_State *L )
{
  int id = ( int )luaL_checkinteger( L, 1 );

  if( id < ELUA_INT_FIRST_ID || id > INT_ELUA_LAST )
    return luaL_error( L, "invalid interrupt ID" );
  lua_rawgeti( L, LUA_REGISTRYINDEX, LUA_INT_HANDLER_KEY );
  lua_rawgeti( L, -1, id );
  return 1;
}

// Lua: flag = get_int_flag( id, resnum, [clear] )
// 'clear' default to true if not specified
static int cpu_get_int_flag( lua_State *L )
{
  elua_int_id id;
  elua_int_resnum resnum;
  int clear = 1;
  int res;

  id = ( elua_int_id )luaL_checkinteger( L, 1 );
  resnum = ( elua_int_resnum )luaL_checkinteger( L, 2 );
  if( lua_gettop( L ) >= 3 )
  {
    if( lua_isboolean( L, 3 ) )
      clear = lua_toboolean( L, 3 );
    else
      return luaL_error( L, "expected a bool as the 3rd argument of this function" );
  }
  res = platform_cpu_get_interrupt_flag( id, resnum, clear );
  if( res == PLATFORM_INT_INVALID )
    return luaL_error( L, "%d is not a valid interrupt ID", ( int )id );
  else if( res == PLATFORM_INT_NOT_HANDLED )
    return luaL_error( L, "get flag operation not implemented for interrupt %d with resource %d", ( int )id, ( int )resnum );
  else if( res == PLATFORM_INT_BAD_RESNUM )
    return luaL_error( L, "resource %d not valid for interrupt %d", ( int )resnum, ( int )id );
  lua_pushinteger( L, res );
  return 1;
}
#endif // #ifdef BUILD_LUA_INT_HANDLERS

// Module function map
#define MIN_OPT_LEVEL 2
#include "lrodefs.h"
const LUA_REG_TYPE cpu_map[] =
{
  { LSTRKEY( "w32" ), LFUNCVAL( cpu_w32 ) },
  { LSTRKEY( "r32" ), LFUNCVAL( cpu_r32 ) },
  { LSTRKEY( "w16" ), LFUNCVAL( cpu_w16 ) },
  { LSTRKEY( "r16" ), LFUNCVAL( cpu_r16 ) },
  { LSTRKEY( "w8" ), LFUNCVAL( cpu_w8 ) },
  { LSTRKEY( "r8" ), LFUNCVAL( cpu_r8 ) },
  { LSTRKEY( "cli" ), LFUNCVAL( cpu_cli ) },
  { LSTRKEY( "sei" ), LFUNCVAL( cpu_sei ) },
  { LSTRKEY( "clock" ), LFUNCVAL( cpu_clock ) },
#ifdef BUILD_LUA_INT_HANDLERS
  { LSTRKEY( "set_int_handler" ), LFUNCVAL( cpu_set_int_handler ) },
  { LSTRKEY( "get_int_handler" ), LFUNCVAL( cpu_get_int_handler ) },
  { LSTRKEY( "get_int_flag" ), LFUNCVAL( cpu_get_int_flag) },
#endif
#if defined( HAS_CPU_CONSTANTS ) && LUA_OPTIMIZE_MEMORY > 0
  { LSTRKEY( "__metatable" ), LROVAL( cpu_map ) },
#endif
#ifdef HAS_CPU_CONSTANTS
  { LSTRKEY( "__index" ), LFUNCVAL( cpu_mt_index ) },
#endif
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_cpu( lua_State *L )
{
#ifdef BUILD_LUA_INT_HANDLERS
  // Create interrupt table
  lua_newtable( L );
  lua_rawseti( L, LUA_REGISTRYINDEX, LUA_INT_HANDLER_KEY );
#endif //#ifdef BUILD_LUA_INT_HANDLERS

#if LUA_OPTIMIZE_MEMORY > 0
  return 0;
#else // #if LUA_OPTIMIZE_MEMORY > 0
  // Register methods
  luaL_register( L, AUXLIB_CPU, cpu_map );

#ifdef HAS_CPU_CONSTANTS
  // Set table as its own metatable
  lua_pushvalue( L, -1 );
  lua_setmetatable( L, -2 );
#endif // #ifdef HAS_CPU_CONSTANTS

  return 1;
#endif // #if LUA_OPTIMIZE_MEMORY > 0
}
