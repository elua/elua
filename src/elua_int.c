// eLua interrupt support

#include "lua.h"
#include "platform.h"
#include "platform_conf.h"
#include "ldebug.h"
#include <stdio.h>
#include <string.h>

// ****************************************************************************
// Lua handlers

#ifdef BUILD_LUA_INT_HANDLERS

// Interrupt queue read and write indexes
static volatile u8 elua_int_read_idx, elua_int_write_idx;
// The interrupt queue
static elua_int_element elua_int_queue[ 1 << PLATFORM_INT_QUEUE_LOG_SIZE ];
// Interrupt enabled/disabled flags
static u32 elua_int_flags[ LUA_INT_MAX_SOURCES / 32 ];

// Masking for read/write indexes
#define INT_IDX_SHIFT                   ( PLATFORM_INT_QUEUE_LOG_SIZE )
#define INT_IDX_MASK                    ( ( 1 << INT_IDX_SHIFT ) - 1 )

// Our hook function (called by the Lua VM)
static void elua_int_hook( lua_State *L, lua_Debug *ar )
{
  elua_int_element crt;
  int old_status;

  // Get interrupt (and remove from queue)
  crt = elua_int_queue[ elua_int_read_idx ];
  elua_int_queue[ elua_int_read_idx ].id = ELUA_INT_EMPTY_SLOT;
  elua_int_read_idx = ( elua_int_read_idx + 1 ) & INT_IDX_MASK;

  if( elua_int_is_enabled( crt.id ) )
  {
    // Call Lua handler
    // Get interrupt handler table
    lua_rawgeti( L, LUA_REGISTRYINDEX, LUA_INT_HANDLER_KEY ); // inttable
    lua_rawgeti( L, -1, crt.id ); // inttable f
    if( !lua_isnil( L, -1 ) )
    {
      lua_pushinteger( L, crt.resnum ); // inttable f resnum
      lua_call( L, 1, 0 ); // inttable    
    }
    else
      lua_remove( L, -1 ); // inttable
    lua_remove( L, -1 );
  }

  old_status = platform_cpu_set_global_interrupts( PLATFORM_CPU_DISABLE );
  if( elua_int_queue[ elua_int_read_idx ].id == ELUA_INT_EMPTY_SLOT ) // no more interrupts in the queue, so clear the hook
    lua_sethook( L, NULL, 0, 0 );
  platform_cpu_set_global_interrupts( old_status );
}

// Queue an interrupt and set the Lua hook
// Returns PLATFORM_OK or PLATFORM_ERR
int elua_int_add( elua_int_id inttype, elua_int_resnum resnum )
{
  if( inttype < ELUA_INT_FIRST_ID || inttype > INT_ELUA_LAST )
    return PLATFORM_ERR;

  // If Lua is not running (no Lua state), or no Lua interrupt handler is set, 
  // or the interrupt is not enabled, don't do anything
  if( lua_getstate() == NULL || !elua_int_is_enabled( inttype ) )
    return PLATFORM_ERR;

  // If there's no more room in the queue, set the overflow flag and return
  if( elua_int_queue[ elua_int_write_idx ].id != ELUA_INT_EMPTY_SLOT )
  {
    printf( "ERROR in elua_int_add: buffer overflow, interrupt not queued\n" );
    return PLATFORM_ERR;
  }

  // Queue the interrupt
  elua_int_queue[ elua_int_write_idx ].id = inttype;
  elua_int_queue[ elua_int_write_idx ].resnum = resnum;
  elua_int_write_idx = ( elua_int_write_idx + 1 ) & INT_IDX_MASK;

  // Set the Lua hook (it's OK to set it even if it's already set)
  lua_sethook( lua_getstate(), elua_int_hook, LUA_MASKCOUNT, 2 ); 

  // All OK
  return PLATFORM_OK;
}

// Enable the given interrupt
void elua_int_enable( elua_int_id inttype )
{
  if( inttype < LUA_INT_MAX_SOURCES )
    elua_int_flags[ inttype >> 5 ] |= 1 << ( inttype & 0x1F );
}

// Disable the given interrupt
void elua_int_disable( elua_int_id inttype )
{
  if( inttype < LUA_INT_MAX_SOURCES )
    elua_int_flags[ inttype >> 5 ] &= ~( 1 << ( inttype & 0x1F ) );
}

// Returns 1 if the given interrupt is enabled, 0 otherwise
int elua_int_is_enabled( elua_int_id inttype )
{
  if( inttype < LUA_INT_MAX_SOURCES )
    return elua_int_flags[ inttype >> 5 ] & ( 1 << ( inttype & 0x1F ) ) ? 1 : 0;
  return 0;
}

// Disable all interrupts (used by Lua in lua_close (lstate.c))
void elua_int_disable_all()
{
  unsigned i;

  for( i = 0; i < LUA_INT_MAX_SOURCES >> 5; i ++ )
    elua_int_flags[ i ] = 0;    
}

// Called from lstate.c/lua_close
void elua_int_cleanup()
{
  elua_int_disable_all();
  elua_int_read_idx = elua_int_write_idx = 0;
  memset( elua_int_queue, ELUA_INT_EMPTY_SLOT, sizeof( elua_int_queue ) );
}

#else // #ifdef BUILD_LUA_INT_HANDLERS

// This is needed by lua_close (lstate.c)
void elua_int_disable_all()
{
}

// This too
void elua_int_cleanup()
{
}

void elua_int_enable( elua_int_id inttype )
{
}

void elua_int_disable( elua_int_id inttype )
{
}

int elua_int_add( elua_int_id inttype, elua_int_resnum resnum )
{
  return PLATFORM_ERR;
}

#endif // #ifdef BUILD_LUA_INT_HANDLERS

// ****************************************************************************
// C handlers

#ifdef BUILD_C_INT_HANDLERS

static elua_int_c_handler elua_int_c_handler_list[ INT_ELUA_LAST ];

elua_int_c_handler elua_int_set_c_handler( elua_int_id inttype, elua_int_c_handler phandler )
{
  elua_int_c_handler crthandler;

  if( inttype < ELUA_INT_FIRST_ID || inttype > INT_ELUA_LAST )
    return NULL;
  inttype -= ELUA_INT_FIRST_ID;
  crthandler = elua_int_c_handler_list[ inttype ];
  elua_int_c_handler_list[ inttype ] = phandler;
  return crthandler;
}

elua_int_c_handler elua_int_get_c_handler( elua_int_id inttype )
{
  if( inttype < ELUA_INT_FIRST_ID || inttype > INT_ELUA_LAST )
    return NULL;
  return elua_int_c_handler_list[ inttype - ELUA_INT_FIRST_ID ];
}

#else // #ifdef BUILD_C_INT_HANDLERS

elua_int_c_handler elua_int_set_c_handler( elua_int_id inttype, elua_int_c_handler phandler )
{
  return NULL;
}

elua_int_c_handler elua_int_get_c_handler( elua_int_id inttype )
{
  return NULL;
}

#endif // #ifdef BUILD_C_INT_HANDLERS

