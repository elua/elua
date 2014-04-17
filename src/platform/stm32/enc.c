// eLua Module for STM32 timer encoder mode support
// enc is a platform-dependent (STM32) module, that binds to Lua the basic API
// from ST

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "lrotable.h"
#include "platform_conf.h"
#include "auxmods.h"
#include "elua_int.h"
#include "enc.h"

static elua_int_c_handler prev_handler;
static elua_int_resnum index_resnum;
static int index_tmr_id;
static u16 index_count;
static void index_handler( elua_int_resnum resnum );

//Lua: init(id)
static int enc_init( lua_State *L )
{
  unsigned id;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( timer, id );
  stm32_enc_init( id );
  return 0;
}

//Lua: setcounter(id, count)
static int enc_set_counter( lua_State *L )
{
  unsigned id, count;

  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( timer, id );
  count = luaL_checkinteger( L, 2 );

  stm32_enc_set_counter( id, count );
  return 0;
}

//Lua: setidxtrig( id, resnum, tmr_id, count )
static int enc_set_index_handler( lua_State *L )
{
  elua_int_id id;
  
  id = ( elua_int_id )luaL_checkinteger( L, 1 );
  if( id < ELUA_INT_FIRST_ID || id > INT_ELUA_LAST )
    return luaL_error( L, "invalid interrupt ID" );
  index_resnum = ( elua_int_resnum )luaL_checkinteger( L, 2 );
  index_tmr_id = luaL_checkinteger( L, 3 );
  MOD_CHECK_ID( timer, index_tmr_id );
  index_count = ( u16 )luaL_checkinteger( L, 4 );

  platform_cpu_set_interrupt( id, index_resnum, PLATFORM_CPU_ENABLE );
  prev_handler = elua_int_set_c_handler( id, index_handler );
  return 0;
}

static void index_handler( elua_int_resnum resnum )
{
  if( prev_handler )
    prev_handler;

  if( resnum != index_resnum )
    return;

  stm32_enc_set_counter( index_tmr_id, index_count );
}


#define MIN_OPT_LEVEL 2
#include "lrodefs.h"  

// Module function map
const LUA_REG_TYPE enc_map[] =
{ 
  { LSTRKEY( "init" ),  LFUNCVAL( enc_init ) },
  { LSTRKEY( "setcounter" ),  LFUNCVAL( enc_set_counter ) },
  { LSTRKEY( "setidxtrig" ),  LFUNCVAL( enc_set_index_handler ) },
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_enc( lua_State *L )
{
  LREGISTER( L, AUXLIB_ENC, enc_map );
}  

