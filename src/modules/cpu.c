// Module for interfacing with CPU

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "auxmods.h"
#include "lrotable.h"
#include <string.h> 

#define _C( x ) { #x, x }
#include "platform_conf.h"

// Lua: w32( address, data )
static int cpu_w32( lua_State *L )
{
  u32 addr, data;
  
  luaL_checkinteger( L, 1 );
  luaL_checkinteger( L, 2 );
  addr = ( u32 )luaL_checknumber( L, 1 );
  data = ( u32 )luaL_checknumber( L, 2 );
  *( u32* )addr = data;
  return 0;
}

// Lua: data = r32( address )
static int cpu_r32( lua_State *L )
{
  u32 addr;

  luaL_checkinteger( L, 1 );
  addr = ( u32 )luaL_checknumber( L, 1 );
  lua_pushinteger( L, ( lua_Integer )( *( u32* )addr ) );  
  return 1;
}

// Lua: w16( address, data )
static int cpu_w16( lua_State *L )
{
  u32 addr;
  u16 data = ( u16 )luaL_checkinteger( L, 2 );
  
  luaL_checkinteger( L, 1 );
  addr = ( u32 )luaL_checknumber( L, 1 );
  *( u16* )addr = data;
  return 0;
}

// Lua: data = r16( address )
static int cpu_r16( lua_State *L )
{
  u32 addr;

  luaL_checkinteger( L, 1 );
  addr = ( u32 )luaL_checknumber( L, 1 );
  lua_pushinteger( L, ( lua_Integer )( *( u16* )addr ) );  
  return 1;
}

// Lua: w8( address, data )
static int cpu_w8( lua_State *L )
{
  u32 addr;
  u8 data = ( u8 )luaL_checkinteger( L, 2 );
  
  luaL_checkinteger( L, 1 );
  addr = ( u32 )luaL_checknumber( L, 1 );
  *( u8* )addr = data;
  return 0;
}

// Lua: data = r8( address )
static int cpu_r8( lua_State *L )
{
  u32 addr;

  luaL_checkinteger( L, 1 );
  addr = ( u32 )luaL_checknumber( L, 1 );
  lua_pushinteger( L, ( lua_Integer )( *( u8* )addr ) );  
  return 1;
}

// Lua: cli()
// Lua: cli() - to disable all interrupts
// or cli( id1, resnum1, [resnum2], ..., [resnumn] ) - to disable a specific id/resnum(s)
static int cpu_cli( lua_State *L )
{
#ifdef BUILD_LUA_INT_HANDLERS
  unsigned i;
  elua_int_id id;
  elua_int_resnum resnum;
  int res;

  if( lua_gettop( L ) > 0 )
  {
    id = ( elua_int_id )luaL_checkinteger( L, 1 );
    for( i = 2; i <= lua_gettop( L ); i ++ )
    {
      resnum = ( elua_int_resnum )luaL_checkinteger( L, i );
      res = platform_cpu_set_interrupt( id, resnum, PLATFORM_CPU_DISABLE );
      if( res == PLATFORM_INT_INVALID )
        return luaL_error( L, "%d is not a valid interrupt ID", ( int )id );
      else if( res == PLATFORM_INT_NOT_HANDLED )
        return luaL_error( L, "cli operation not implemented for interrupt %d with resource %d", ( int )id, ( int )resnum );
      else if( res == PLATFORM_INT_BAD_RESNUM )
        return luaL_error( L, "resource %d not valid for interrupt %d", ( int )resnum, ( int )id );
    }
    elua_int_disable( id );
  }
  else
#else // #ifdef BUILD_LUA_INT_HANDLERS
  if( lua_gettop( L ) > 0 )
    return luaL_error( L, "Lua interrupt support not available." );
#endif // #ifdef BUILD_LUA_INT_HANDLERS
  platform_cpu_set_global_interrupts( PLATFORM_CPU_DISABLE );
  return 0;
}


// Lua: sei() - to enable all interrupts
// or sei( id1, resnum1, [resnum2], ..., [resnumn] ) - to enable a specific id/resnum(s)
static int cpu_sei( lua_State *L )
{
#ifdef BUILD_LUA_INT_HANDLERS  
  unsigned i;
  elua_int_id id;
  elua_int_resnum resnum;  
  int res;

  if( lua_gettop( L ) > 0 )
  {
    id = ( elua_int_id )luaL_checkinteger( L, 1 );
    for( i = 2; i <= lua_gettop( L ); i ++ )
    {
      resnum = ( elua_int_resnum )luaL_checkinteger( L, i );
      res = platform_cpu_set_interrupt( id, resnum, PLATFORM_CPU_ENABLE );
      if( res == PLATFORM_INT_INVALID )
        return luaL_error( L, "%d is not a valid interrupt ID", ( int )id );
      else if( res == PLATFORM_INT_NOT_HANDLED )
        return luaL_error( L, "sei operation not implemented for interrupt %d with resnum %d", ( int )id, ( int )resnum );
      else if( res == PLATFORM_INT_BAD_RESNUM )
        return luaL_error( L, "resource %d not valid for interrupt %d", ( int )resnum, ( int )id );
    }
    elua_int_enable( id );
  }
  else
#else // #ifdef BUILD_LUA_INT_HANDLERS
  if( lua_gettop( L ) > 0 )
    return luaL_error( L, "Lua interrupt support not available." );
#endif // #ifdef BUILD_LUA_INT_HANDLERS  
  platform_cpu_set_global_interrupts( PLATFORM_CPU_ENABLE );
  return 0;
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

#ifdef PLATFORM_CPU_CONSTANTS
static const cpu_const_t cpu_constants[] = 
{
  PLATFORM_CPU_CONSTANTS,
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
#endif

#ifdef BUILD_LUA_INT_HANDLERS
static u8 cpu_int_handler_active;

u8 cpu_is_int_handler_active()
{
  return cpu_int_handler_active;
}

// lua: cpu.set_int_handler( f )
static int cpu_set_int_handler( lua_State *L )
{
  if( lua_type( L, 1 ) == LUA_TNIL )
    cpu_int_handler_active = 0;
  else if( lua_type( L, 1 ) == LUA_TFUNCTION || lua_type( L, 1 ) == LUA_TLIGHTFUNCTION )
  {
    lua_settop( L, 1 );
    lua_rawseti( L, LUA_REGISTRYINDEX, LUA_INT_HANDLER_KEY );
    cpu_int_handler_active = 1;
  }
  else
    return luaL_error( L, "invalid argument (must be a function or nil)" );
  return 0;
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
  { LSTRKEY( "get_int_flag" ), LFUNCVAL( cpu_get_int_flag) },
  { LSTRKEY( "INT_FLAG_CLEAR" ), LBOOLVAL( 1 ) },
  { LSTRKEY( "INT_FLAG_KEEP" ), LBOOLVAL( 0 ) },
#endif
#if defined( PLATFORM_CPU_CONSTANTS ) && LUA_OPTIMIZE_MEMORY > 0
  { LSTRKEY( "__metatable" ), LROVAL( cpu_map ) },
#endif
#ifdef PLATFORM_CPU_CONSTANTS
  { LSTRKEY( "__index" ), LFUNCVAL( cpu_mt_index ) },
#endif
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_cpu( lua_State *L )
{
#ifdef BUILD_LUA_INT_HANDLERS
  cpu_int_handler_active = 0;
#endif
#if LUA_OPTIMIZE_MEMORY > 0
  return 0;
#else // #if LUA_OPTIMIZE_MEMORY > 0
  // Register methods
  luaL_register( L, AUXLIB_CPU, cpu_map );
  
#ifdef PLATFORM_CPU_CONSTANTS
  // Set table as its own metatable
  lua_pushvalue( L, -1 );
  lua_setmetatable( L, -2 );
#endif // #ifdef PLATFORM_CPU_CONSTANTS
  
  return 1;
#endif // #if LUA_OPTIMIZE_MEMORY > 0
}
