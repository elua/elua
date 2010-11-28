// eLua dynamic loader test module

#include <stdio.h>
#include <stdlib.h>
#include "lua.h"
#include "lauxlib.h"
#include "type.h"

#define UDL_MODNAME           "test"

typedef u32 ( *p_udl_get_offset )( int );
extern int udl_get_id();
#define UDL_MOD_FUNC          __attribute__ ((section(".modtext")))
#define UDL_PTR( x )          ( ( x ) + udl_get_offset( udl_id ) )
#define UDL_FUNC_PTR( x )     ( ( u32 )( x ) + udl_get_offset( udl_id ) + 1 )
#define UDL_DECL_ID           int udl_id = udl_get_id()
#define UDL_DECL_GET_OFFSET   p_udl_get_offset udl_get_offset = ( p_udl_get_offset )*( u32* )0x20000008
#define UDL_FUNC_DECL\
  int udl_id = udl_get_id();\
  p_udl_get_offset udl_get_offset = ( p_udl_get_offset )*( u32* )0x20000008

UDL_MOD_FUNC static int test_func( lua_State *L )
{
  UDL_FUNC_DECL;
  const char *parg = luaL_checkstring( L, 1 );

  printf( UDL_PTR( "Called with argument '%s'\n" ), parg );
  lua_pushinteger( L, 666 );
  return 1;
}

UDL_MOD_FUNC int luaopen_testmod( lua_State *L )
{
  UDL_FUNC_DECL;
  struct luaL_Reg ftable[ 2 ];

  printf( UDL_PTR( "luaopen_testmod called: func offset is %08X, full ptr is %08X\n" ), ( u32 )test_func, UDL_PTR( test_func ) );
  ftable[ 0 ].name = UDL_PTR( "testfunc" );
  ftable[ 0 ].func = ( lua_CFunction )UDL_PTR( test_func );
  ftable[ 1 ].name = ftable[ 1 ].func = NULL;
  luaL_register( L, UDL_PTR( UDL_MODNAME ), ftable );
  return 1;
}

