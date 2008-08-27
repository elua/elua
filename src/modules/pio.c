// Module for interfacing with PIO

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "auxmods.h"

// Local operation masks for all the ports
static pio_type pio_masks[ PLATFORM_IO_PORTS ];

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
      platform_pio_op( i, pio_masks[ i ], op );
  return 0;
}

// Helper function: port operations
// Gets the stack index of the first port and the operation (also the mask)
static int pioh_set_ports( lua_State* L, int stackidx, int op, pio_type mask )
{
  int total = lua_gettop( L );
  int i, v, port;
  
  pioh_clear_masks();
  
  // Get all masks
  for( i = stackidx; i <= total; i ++ )
  {
    v = luaL_checkinteger( L, i );
    port = PLATFORM_IO_GET_PORT( v );
    if( !PLATFORM_IO_IS_PORT( v ) || !platform_pio_has_port( port ) )
      return luaL_error( L, "invalid port" );
    pio_masks[ port ] = mask;
  }
  
  // Ask platform to execute the given operation
  for( i = 0; i < PLATFORM_IO_PORTS; i ++ )
    if( pio_masks[ i ] )
      platform_pio_op( i, pio_masks[ i ], op );
  return 0;
}

// Lua: setpin( val, Pin1, Pin2, Pin3 ... )
static int pio_set_pin_state( lua_State* L )
{
  int value;
  
  value = luaL_checkinteger( L, 1 );  
  return pioh_set_pins( L, 2, value ? PLATFORM_IO_PIN_SET : PLATFORM_IO_PIN_CLEAR );
}

// Lua: set( Pin1, Pin2, Pin3 ... )
static int pio_set_pin( lua_State* L )
{
  return pioh_set_pins( L, 1, PLATFORM_IO_PIN_SET );
}

// Lua: clear( Pin1, Pin2, Pin3 ... )
static int pio_clear_pin( lua_State* L )
{
  return pioh_set_pins( L, 1, PLATFORM_IO_PIN_CLEAR );
}

// Lua: input( Pin1, Pin2, Pin3 ... )
static int pio_pin_input( lua_State* L )
{
  return pioh_set_pins( L, 1, PLATFORM_IO_PIN_DIR_INPUT );
}

// Lua: output( Pin1, Pin2, Pin3 ... )
static int pio_pin_output( lua_State* L )
{
  return pioh_set_pins( L, 1, PLATFORM_IO_PIN_DIR_OUTPUT );
}

// Lua: setport( val, Port1, Port2, ... )
static int pio_set_port( lua_State* L )
{
  int value;
  
  value = luaL_checkinteger( L, 1 );  
  return pioh_set_ports( L, 2, PLATFORM_IO_PORT_SET_VALUE, value );
}

// Lua: value1, value2, ... = getport( Port1, Port2, ... )
static int pio_get_port( lua_State* L )
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

// Lua: port_input( Port1, Port2, ... )
static int pio_port_input( lua_State* L )
{
  return pioh_set_ports( L, 1, PLATFORM_IO_PORT_DIR_INPUT, PLATFORM_IO_ALL_PINS );
}

// Lua: port_output( Port1, Port2, ... )
static int pio_port_output( lua_State* L )
{
  return pioh_set_ports( L, 1, PLATFORM_IO_PORT_DIR_OUTPUT, PLATFORM_IO_ALL_PINS );
}

// Lua: value1, value2, ... = get( Pin1, Pin2 ... )
static int pio_get_pin( lua_State* L )
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
      value = platform_pio_op( port, PLATFORM_IO_PIN_GET, 1 << pin );
      lua_pushinteger( L, value );
    }
  }
  return total;
}

// Lua: port = port( value )
static int pio_port( lua_State* L )
{
  pio_type value = ( pio_type )luaL_checkinteger( L, 1 );
  
  lua_pushinteger( L, PLATFORM_IO_GET_PORT( value ) );
  return 1;
}

// Lua: pin = pin( value )
static int pio_pin( lua_State* L )
{
  pio_type value = ( pio_type )luaL_checkinteger( L, 1 );
  
  lua_pushinteger( L, PLATFORM_IO_GET_PIN( value ) );
  return 1;
}

// Module function map
static const luaL_reg pio_map[] = 
{
  { "setpin",  pio_set_pin_state },
  { "set", pio_set_pin },
  { "get", pio_get_pin },
  { "clear", pio_clear_pin },
  { "input", pio_pin_input },
  { "output", pio_pin_output },
  { "setport", pio_set_port },
  { "getport", pio_get_port },
  { "port_input", pio_port_input },
  { "port_output", pio_port_output },
  { "port", pio_port },
  { "pin", pio_pin },
  { NULL, NULL }
};

LUALIB_API int luaopen_pio( lua_State *L )
{
  unsigned port, pin;
  char name[ 10 ];
  
  luaL_register( L, AUXLIB_PIO, pio_map );
  
  // Add all port/pins combinations to our module
  for( port = 0; port < PLATFORM_IO_PORTS; port ++ )
    if( platform_pio_has_port( port ) )
    {
      // First the whole port
      sprintf( name, "%s", platform_pio_get_prefix( port ) );
      lua_pushnumber( L, ( lua_Number )PLATFORM_IO_ENCODE( port, 0, PLATFORM_IO_ENC_PORT ) );
      lua_setfield( L, -2, name );        
      // Then all its pins
      for( pin = 0; pin < PLATFORM_IO_PINS; pin ++ )
        if( platform_pio_has_pin( port, pin ) )
        {
          sprintf( name, "%s_%d", platform_pio_get_prefix( port ), pin );
          lua_pushnumber( L, ( lua_Number )PLATFORM_IO_ENCODE( port, pin, PLATFORM_IO_ENC_PIN ) );
          lua_setfield( L, -2, name );
        }
    }
  return 1;
}
