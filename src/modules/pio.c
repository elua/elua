// Module for interfacing with PIO

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "auxmods.h"
#include "lrotable.h"
#include "platform_conf.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// PIO public constants
#define PIO_DIR_OUTPUT      0
#define PIO_DIR_INPUT       1

// PIO private constants
#define PIO_PORT_OP         0
#define PIO_PIN_OP          1

// Local operation masks for all the ports
static pio_type pio_masks[ PLATFORM_IO_PORTS ];

// ****************************************************************************
// Generic helper functions

// Helper function: clear all masks
static void pioh_clear_masks()
{
  int i;
  
  for( i = 0; i < PLATFORM_IO_PORTS; i ++ )
    pio_masks[ i ] = 0;
}

// Helper function: pin operations
// Gets the stack index of the first pin and the operation
static int pioh_set_pins( lua_State* L, int stackidx, int op )
{
  int total = lua_gettop( L );
  int i, v, port, pin;
  
  pioh_clear_masks();
  
  // Get all masks
  for( i = stackidx; i <= total; i ++ )
  {
    v = luaL_checkinteger( L, i );
    port = PLATFORM_IO_GET_PORT( v );
    pin = PLATFORM_IO_GET_PIN( v );
    if( PLATFORM_IO_IS_PORT( v ) || !platform_pio_has_port( port ) || !platform_pio_has_pin( port, pin ) )
      return luaL_error( L, "invalid pin" );
    pio_masks[ port ] |= 1 << pin;
  }
  
  // Ask platform to execute the given operation
  for( i = 0; i < PLATFORM_IO_PORTS; i ++ )
    if( pio_masks[ i ] )
      if( !platform_pio_op( i, pio_masks[ i ], op ) )
        return luaL_error( L, "invalid PIO operation" );
  return 0;
}

// Helper function: port operations
// Gets the stack index of the first port and the operation (also the mask)
static int pioh_set_ports( lua_State* L, int stackidx, int op, pio_type mask )
{
  int total = lua_gettop( L );
  int i, v, port;
  u32 port_mask = 0;

  // Get all masks
  for( i = stackidx; i <= total; i ++ )
  {
    v = luaL_checkinteger( L, i );
    port = PLATFORM_IO_GET_PORT( v );
    if( !PLATFORM_IO_IS_PORT( v ) || !platform_pio_has_port( port ) )
      return luaL_error( L, "invalid port" );
    port_mask |= ( 1ULL << port );
  }
  
  // Ask platform to execute the given operation
  for( i = 0; i < PLATFORM_IO_PORTS; i ++ )
    if( port_mask & ( 1ULL << i ) )
      if( !platform_pio_op( i, mask, op ) )
        return luaL_error( L, "invalid PIO operation" );
  return 0;
}

// ****************************************************************************
// Pin/port helper functions

static int pio_gen_setdir( lua_State *L, int optype )
{
  int op = luaL_checkinteger( L, 1 );

  if( op == PIO_DIR_INPUT )
    op = optype == PIO_PIN_OP ? PLATFORM_IO_PIN_DIR_INPUT : PLATFORM_IO_PORT_DIR_INPUT;
  else if( op == PIO_DIR_OUTPUT )
    op = optype == PIO_PIN_OP ? PLATFORM_IO_PIN_DIR_OUTPUT : PLATFORM_IO_PORT_DIR_OUTPUT;
  else
    return luaL_error( L, "invalid direction" );
  if( optype == PIO_PIN_OP )
    pioh_set_pins( L, 2, op );
  else
    pioh_set_ports( L, 2, op, PLATFORM_IO_ALL_PINS );
  return 0;
}

static int pio_gen_setpull( lua_State *L, int optype )
{
  int op = luaL_checkinteger( L, 1 );

  if( ( op != PLATFORM_IO_PIN_PULLUP ) &&
      ( op != PLATFORM_IO_PIN_PULLDOWN ) &&
      ( op != PLATFORM_IO_PIN_NOPULL ) )
    return luaL_error( L, "invalid pull type" );
  if( optype == PIO_PIN_OP )
    pioh_set_pins( L, 2, op );
  else
    pioh_set_ports( L, 2, op, PLATFORM_IO_ALL_PINS );
  return 0;
}

static int pio_gen_setval( lua_State *L, int optype, pio_type val, int stackidx )
{
  if( ( optype == PIO_PIN_OP ) && ( val != 1 ) && ( val != 0 ) ) 
    return luaL_error( L, "invalid pin value" );
  if( optype == PIO_PIN_OP )
    pioh_set_pins( L, stackidx, val == 1 ? PLATFORM_IO_PIN_SET : PLATFORM_IO_PIN_CLEAR );
  else
    pioh_set_ports( L, stackidx, PLATFORM_IO_PORT_SET_VALUE, val );
  return 0;
}

// ****************************************************************************
// Pin operations

// Lua: pio.pin.setdir( pio.INPUT | pio.OUTPUT, pin1, pin2, ..., pinn )
static int pio_pin_setdir( lua_State *L )
{
  return pio_gen_setdir( L, PIO_PIN_OP );
}

// Lua: pio.pin.setpull( pio.PULLUP | pio.PULLDOWN | pio.NOPULL, pin1, pin2, ..., pinn )
static int pio_pin_setpull( lua_State *L )
{
  return pio_gen_setpull( L, PIO_PIN_OP );
}

// Lua: pio.pin.setval( 0|1, pin1, pin2, ..., pinn )
static int pio_pin_setval( lua_State *L )
{
  pio_type val = ( pio_type )luaL_checkinteger( L, 1 );

  return pio_gen_setval( L, PIO_PIN_OP, val, 2 );
}

// Lua: pio.pin.sethigh( pin1, pin2, ..., pinn )
static int pio_pin_sethigh( lua_State *L )
{
  return pio_gen_setval( L, PIO_PIN_OP, 1, 1 );
}

// Lua: pio.pin.setlow( pin1, pin2, ..., pinn )
static int pio_pin_setlow( lua_State *L )
{
  return pio_gen_setval( L, PIO_PIN_OP, 0, 1 );
}

// Lua: pin1, pin2, ..., pinn = pio.pin.getval( pin1, pin2, ..., pinn )
static int pio_pin_getval( lua_State *L )
{
  pio_type value;
  int v, i, port, pin;
  int total = lua_gettop( L );
  
  for( i = 1; i <= total; i ++ )
  {
    v = luaL_checkinteger( L, i );  
    port = PLATFORM_IO_GET_PORT( v );
    pin = PLATFORM_IO_GET_PIN( v );
    if( PLATFORM_IO_IS_PORT( v ) || !platform_pio_has_port( port ) || !platform_pio_has_pin( port, pin ) )
      return luaL_error( L, "invalid pin" );
    else
    {
      value = platform_pio_op( port, 1 << pin, PLATFORM_IO_PIN_GET );
      lua_pushinteger( L, value );
    }
  }
  return total;
}

// ****************************************************************************
// Port operations

// Lua: pio.port.setdir( pio.INPUT | pio.OUTPUT, port1, port2, ..., portn )
static int pio_port_setdir( lua_State *L )
{
  return pio_gen_setdir( L, PIO_PORT_OP );
}

// Lua: pio.port.setpull( pio.PULLUP | pio.PULLDOWN | pio.NOPULL, port1, port2, ..., portn )
static int pio_port_setpull( lua_State *L )
{
  return pio_gen_setpull( L, PIO_PORT_OP );
}

// Lua: pio.port.setval( value, port1, port2, ..., portn )
static int pio_port_setval( lua_State *L )
{
  pio_type val = ( pio_type )luaL_checkinteger( L, 1 );

  return pio_gen_setval( L, PIO_PORT_OP, val, 2 );
}

// Lua: pio.port.sethigh( port1, port2, ..., portn )
static int pio_port_sethigh( lua_State *L )
{
  return pio_gen_setval( L, PIO_PORT_OP, PLATFORM_IO_ALL_PINS, 1 );
}

// Lua: pio.port.setlow( port1, port2, ..., portn )
static int pio_port_setlow( lua_State *L )
{
  return pio_gen_setval( L, PIO_PORT_OP, 0, 1 );
}

// Lua: val1, val2, ..., valn = pio.port.getval( port1, port2, ..., portn )
static int pio_port_getval( lua_State *L )
{
  pio_type value;
  int v, i, port;
  int total = lua_gettop( L );
  
  for( i = 1; i <= total; i ++ )
  {
    v = luaL_checkinteger( L, i );  
    port = PLATFORM_IO_GET_PORT( v );
    if( !PLATFORM_IO_IS_PORT( v ) || !platform_pio_has_port( port ) )
      return luaL_error( L, "invalid port" );
    else
    {
      value = platform_pio_op( port, PLATFORM_IO_ALL_PINS, PLATFORM_IO_PORT_GET_VALUE );
      lua_pushinteger( L, value );
    }
  }
  return total;
}

// ****************************************************************************
// The __index metamethod will return pin/port numeric identifiers

static int pio_mt_index( lua_State* L )
{
  const char *key = luaL_checkstring( L ,2 );
  int port = 0xFFFF, pin = 0xFFFF, isport = 0, sz;
  
  if( !key || *key != 'P' )
    return 0;
  if( isupper( key[ 1 ] ) ) // PA, PB, ...
  {
    if( PIO_PREFIX != 'A' )
      return 0;
    port = key[ 1 ] - 'A';
    if( key[ 2 ] == '\0' )
      isport = 1;
    else if( key[ 2 ] == '_' )      
      if( sscanf( key + 3, "%d%n", &pin, &sz ) != 1 || sz != strlen( key ) - 3 )
        return 0;      
  }
  else // P0, P1, ...
  {
    if( PIO_PREFIX != '0' )
      return 0;
    if( !strchr( key, '_' ) )   // parse port
    {
      if( sscanf( key + 1, "%d%n", &port, &sz ) != 1  || sz != strlen( key ) - 1 )
        return 0;
      isport = 1;
    }
    else    // parse port_pin
      if( sscanf( key + 1, "%d_%d%n", &port, &pin, &sz ) != 2 || sz != strlen( key ) - 1 )
        return 0;
  }
  sz = -1;
  if( isport )
  {
    if( platform_pio_has_port( port ) )
      sz = PLATFORM_IO_ENCODE( port, 0, 1 );
  }
  else
  {
    if( platform_pio_has_port( port ) && platform_pio_has_pin( port, pin ) )
      sz = PLATFORM_IO_ENCODE( port, pin, 0 );
  }
  if( sz == -1 )
    return 0;
  else
  {
    lua_pushinteger( L, sz );
    return 1;
  }
}

// *****************************************************************************
// The 'decode' functions returns a port/pin pair from a platform code

static int pio_decode( lua_State *L )
{
  int code = ( int )luaL_checkinteger( L, 1 );
  
  lua_pushinteger( L, PLATFORM_IO_GET_PORT( code ) );
  lua_pushinteger( L, PLATFORM_IO_GET_PIN( code ) );
  return 2;
}

// *****************************************************************************
// Pin function map

#define MIN_OPT_LEVEL 2
#include "lrodefs.h"
static const LUA_REG_TYPE pio_pin_map[] =
{
  { LSTRKEY( "setdir" ), LFUNCVAL ( pio_pin_setdir ) },
  { LSTRKEY( "setpull" ), LFUNCVAL( pio_pin_setpull ) },
  { LSTRKEY( "setval" ), LFUNCVAL( pio_pin_setval ) },
  { LSTRKEY( "sethigh" ), LFUNCVAL( pio_pin_sethigh ) },
  { LSTRKEY( "setlow" ), LFUNCVAL( pio_pin_setlow ) },
  { LSTRKEY( "getval" ), LFUNCVAL( pio_pin_getval ) },
  { LNILKEY, LNILVAL }
};

static const LUA_REG_TYPE pio_port_map[] =
{
  { LSTRKEY( "setdir" ), LFUNCVAL ( pio_port_setdir ) },
  { LSTRKEY( "setpull" ), LFUNCVAL( pio_port_setpull ) },
  { LSTRKEY( "setval" ), LFUNCVAL( pio_port_setval ) },
  { LSTRKEY( "sethigh" ), LFUNCVAL( pio_port_sethigh ) },
  { LSTRKEY( "setlow" ), LFUNCVAL( pio_port_setlow ) },
  { LSTRKEY( "getval" ), LFUNCVAL( pio_port_getval ) },
  { LNILKEY, LNILVAL }
};

const LUA_REG_TYPE pio_map[] =
{
#if LUA_OPTIMIZE_MEMORY > 0
  { LSTRKEY( "pin" ), LROVAL( pio_pin_map ) },
  { LSTRKEY( "port" ), LROVAL( pio_port_map ) },
  { LSTRKEY( "decode" ), LFUNCVAL( pio_decode ) },  
  { LSTRKEY( "INPUT" ), LNUMVAL( PIO_DIR_INPUT ) },
  { LSTRKEY( "OUTPUT" ), LNUMVAL( PIO_DIR_OUTPUT ) },
  { LSTRKEY( "PULLUP" ), LNUMVAL( PLATFORM_IO_PIN_PULLUP ) },
  { LSTRKEY( "PULLDOWN" ), LNUMVAL( PLATFORM_IO_PIN_PULLDOWN ) },
  { LSTRKEY( "NOPULL" ), LNUMVAL( PLATFORM_IO_PIN_NOPULL ) },
  { LSTRKEY( "__metatable" ), LROVAL( pio_map ) },
#endif
  { LSTRKEY( "__index" ), LFUNCVAL( pio_mt_index ) },
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_pio( lua_State *L )
{
#if LUA_OPTIMIZE_MEMORY > 0
  return 0;
#else // #if LUA_OPTIMIZE_MEMORY > 0
  luaL_register( L, AUXLIB_PIO, pio_map );

  // Set it as its own metatable
  lua_pushvalue( L, -1 );
  lua_setmetatable( L, -2 );
  
  // Set constants for direction/pullups
  MOD_REG_NUMBER( L, "INPUT", PIO_DIR_INPUT );
  MOD_REG_NUMBER( L, "OUTPUT", PIO_DIR_OUTPUT );
  MOD_REG_NUMBER( L, "PULLUP", PLATFORM_IO_PIN_PULLUP );
  MOD_REG_NUMBER( L, "PULLDOWN", PLATFORM_IO_PIN_PULLDOWN );
  MOD_REG_NUMBER( L, "NOPULL", PLATFORM_IO_PIN_NOPULL );

  // Setup the new tables (pin and port) inside pio
  lua_newtable( L );
  luaL_register( L, NULL, pio_pin_map );
  lua_setfield( L, -2, "pin" );

  lua_newtable( L );
  luaL_register( L, NULL, pio_port_map );
  lua_setfield( L, -2, "port" );

  return 1;
#endif // #if LUA_OPTIMIZE_MEMORY > 0
}

