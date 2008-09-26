// Module for interfacing with CPU

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "auxmods.h"
#include <string.h> 

#define _C( x ) { #x, x }
#include "platform_cpu.h"

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

static const cpu_const_t cpu_constants[] = 
{
#ifdef PLATFORM_CPU_CONSTANTS
  PLATFORM_CPU_CONSTANTS,
#endif
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

// Metatable data
static const luaL_reg cpu_mt_map[] =
{
  { "__index", cpu_mt_index },
  { NULL, NULL }
};

// Module function map
static const luaL_reg cpu_map[] = 
{
  { "w32", cpu_w32 },
  { "r32", cpu_r32 },
  { "w16", cpu_w16 },
  { "r16", cpu_r16 },
  { "w8", cpu_w8 },
  { "r8", cpu_r8 },
  { "cli", cpu_cli },
  { "sei", cpu_sei },
  { "clock", cpu_clock },
  { NULL, NULL }
};

LUALIB_API int luaopen_cpu( lua_State *L )
{
  // Register methods
  luaL_register( L, AUXLIB_CPU, cpu_map );
  
  // Create and set metatable
  lua_newtable( L );
  luaL_register( L, NULL, cpu_mt_map );  
  lua_setmetatable( L, -2 );  
  
  return 1;
}
