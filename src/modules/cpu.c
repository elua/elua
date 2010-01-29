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
  u32 addr = ( u32 )luaL_checkinteger( L, 1 );
  u32 data = ( u32 )luaL_checkinteger( L, 2 );
  
  *( u32* )addr = data;
  return 0;
}

// Lua: data = r32( address )
static int cpu_r32( lua_State *L )
{
  u32 addr = ( u32 )luaL_checkinteger( L, 1 );

  lua_pushinteger( L, ( lua_Integer )( *( u32* )addr ) );  
  return 1;
}

// Lua: w16( address, data )
static int cpu_w16( lua_State *L )
{
  u32 addr = ( u32 )luaL_checkinteger( L, 1 );
  u16 data = ( u16 )luaL_checkinteger( L, 2 );
  
  *( u16* )addr = data;
  return 0;
}

// Lua: data = r16( address )
static int cpu_r16( lua_State *L )
{
  u32 addr = ( u32 )luaL_checkinteger( L, 1 );

  lua_pushinteger( L, ( lua_Integer )( *( u16* )addr ) );  
  return 1;
}

// Lua: w8( address, data )
static int cpu_w8( lua_State *L )
{
  u32 addr = ( u32 )luaL_checkinteger( L, 1 );
  u8 data = ( u8 )luaL_checkinteger( L, 2 );
  
  *( u8* )addr = data;
  return 0;
}

// Lua: data = r8( address )
static int cpu_r8( lua_State *L )
{
  u32 addr = ( u32 )luaL_checkinteger( L, 1 );

  lua_pushinteger( L, ( lua_Integer )( *( u8* )addr ) );  
  return 1;
}

// Lua: cli()
static int cpu_cli( lua_State *L )
{
  platform_cpu_disable_interrupts();
  return 0;
}

// Lua: sei()
static int cpu_sei( lua_State *L )
{
  platform_cpu_enable_interrupts();
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
      lua_pushinteger( L, cpu_constants[ i ].val );
      return 1;
    }
    i ++;
  }
  return 0;
}
#endif

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
