// eLua interrupt support

#include "elua_int.h"
#include "lua.h"
#include "platform.h"
#include "platform_conf.h"
#include "type.h"
#include "ldebug.h"

#ifdef BUILD_LUA_INT_HANDLERS

// 1 if the interrupt queue has overflowed, 0 otherwise
static u8 elua_int_overflow_flag;
// Interrupt queue read and write indexes
static volatile u8 elua_int_read_idx, elua_int_write_idx;
// Number of interrupts in queue
static volatile u8 elua_int_queue_size;
// The interrupt queue
static elua_int_id elua_int_queue[ PLATFORM_INT_QUEUE_SIZE ];
// Interrupt enabled/disabled flags
static u32 elua_int_flags[ LUA_INT_MAX_SOURCES / 32 ];

// We need to know if there is a global interrupt handler set in Lua
// (this comes from src/modules/cpu.c)
extern u8 cpu_is_int_handler_active();

// Our hook function (called by the Lua VM)
static void elua_int_hook( lua_State *L, lua_Debug *ar )
{
  elua_int_id crt;

  // Get interrupt (and remove from queue)
  crt = elua_int_queue[ elua_int_read_idx ];
  // [TODO] remove the modulo operations, replace them with shifts
  elua_int_read_idx = ( elua_int_read_idx + 1 ) % PLATFORM_INT_QUEUE_SIZE;
  // [TODO] is it right to enable/disable interrupts like this? What if they are already disabled somewher else?
  // A store/restore sequence is more adequate
  platform_cpu_disable_interrupts();
  elua_int_queue_size --;
  platform_cpu_enable_interrupts();

  // Call Lua handler
  if( cpu_is_int_handler_active() )
  {
    lua_rawgeti( L, LUA_REGISTRYINDEX, LUA_INT_HANDLER_KEY );
    lua_pushinteger( L, crt );
    lua_call( L, 1, 0 );
  }

  // Unset hook if needed
  platform_cpu_disable_interrupts();
  if( elua_int_queue_size == 0 )
    lua_sethook( L, elua_int_hook, 0, 0 );
  platform_cpu_enable_interrupts();
}

// Queue an interrupt and set the Lua hook
// Returns PLATFORM_OK or PLATFORM_ERR
int elua_int_add( elua_int_id inttype )
{
  // If Lua is not running (no Lua state), or no Lua interrupt handler is set, 
  // or the interrupt is not enabled, don't do anything
  if( lua_getstate() == NULL || !cpu_is_int_handler_active() || !elua_int_is_enabled( inttype ) )
    return PLATFORM_ERR;

  // If there's no more room in the queue, set the overflow flag and return
  if( elua_int_queue_size == PLATFORM_INT_QUEUE_SIZE )
  {
    elua_int_overflow_flag = 1;
    return PLATFORM_ERR;
  }

  // Queue the interrupt
  elua_int_queue[ elua_int_write_idx ] = inttype;
  elua_int_write_idx = ( elua_int_write_idx + 1 ) % PLATFORM_INT_QUEUE_SIZE;
  elua_int_queue_size ++;

  // Set the Lua hook (it's OK to set it even if it's already set)
  // [TODO] is it safe to call lua_sethook here ? If not, set a "trap" in lvm.c
  lua_sethook( lua_getstate(), elua_int_hook, LUA_MASKCALL | LUA_MASKRET | LUA_MASKLINE | LUA_MASKCOUNT, 2 ); 

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

#else // #ifdef BUILD_LUA_INT_HANDLERS

// This is needed by lua_close (lstate.c)
void elua_int_disable_all()
{
}

#endif // #ifdef BUILD_LUA_INT_HANDLERS

