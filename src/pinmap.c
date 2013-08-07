// Pin mapping functions

#include "platform_conf.h"
#ifdef HAS_PINMAPS 

#define IMPLEMENT_PINMAPS
#include "platform.h"
#include "pinmap.h"
#include ELUA_CPU_PINMAP_HEADER
#include <stdarg.h>

///////////////////////////////////////////////////////////////////////////////
// Local/private functions

static const pin_function* pinmaph_find_pin( int pin )
{
  const pin_info* pinfo = pinfuncs;

  if( pin == IGNORE_PIN )
    return NULL;
  // [TODO] this should be a binary search?
  while( pinfo->pin != NO_PIN )
  {
    if( pinfo->pin == ( u16 )pin )
      return platform_pio_has_pin( PLATFORM_IO_GET_PORT( pinfo->pin ), PLATFORM_IO_GET_PIN( pinfo->pin ) ) ? NULL : pinfo->pfuncs;
    pinfo ++;
  }
  return NULL;
}

static int pinmaph_find_function( const pin_function* pi, int peripheral_id, int id, int pin_id )
{
  while( pi->peripheral != PINMAP_NONE )
  {
    if( pi->peripheral == peripheral_id && pi->id == id && pi->pin_id == pin_id )
      return pi->pin_data;
    pi ++;
  }
  return -1;
}

static int pinmaph_create_mapping( int peripheral_id, int id, int num_pins, ... )
{
  int pins[ num_pins ];
  pin_be_info pin_info[ num_pins ];
  int i, temp;
  va_list va;
  const pin_function* pf;

  va_start( va, num_pins );
  for( i = 0; i < num_pins; i ++ )
  {
    if( ( pins[ i ] = va_arg( va, int ) ) == IGNORE_PIN )
      continue;
    if( ( pf = pinmaph_find_pin( pins[ i ] ) ) == NULL )
      return 0;
    if( ( temp = pinmaph_find_function( pf, peripheral_id, id, i ) ) == -1 )
      return 0;
    pin_info[ i ] = ( pin_be_info )temp;
  }
  for( i = 0; i < num_pins; i ++ )
    if( pins[ i ] != IGNORE_PIN )
      platform_pio_set_function( PLATFORM_IO_GET_PORT( pins[ i ] ), PLATFORM_IO_GET_PIN( pins[ i ] ), pin_info[ i ] );
  va_end( va );
  return 1;
}

////////////////////////////////////////////////////////////////////////////////
// Public interface

int pinmap_uart( int id, int rx, int tx, int rts, int cts )
{
  if( id >= NUM_UART )
    return 0;
  return pinmaph_create_mapping( PINMAP_UART, id, 4, rx, tx, rts, cts );
}

int pinmap_initial_config()
{
#if NUM_UART > 0 && defined( UART0_PINMAP )
  pinmap_uart( 0, UART0_RX_PIN, UART0_TX_PIN, UART0_RTS_PIN, UART0_CTS_PIN );
#endif
#if NUM_UART > 1 && defined( UART1_PINMAP )
  pinmap_uart( 1, UART1_RX_PIN, UART1_TX_PIN, UART1_RTS_PIN, UART1_CTS_PIN );
#endif
#if NUM_UART > 2 && defined( UART2_PINMAP )
  pinmap_uart( 2, UART2_RX_PIN, UART2_TX_PIN, UART2_RTS_PIN, UART2_CTS_PIN );
#endif
#if NUM_UART > 3 && defined( UART3_PINMAP )
  pinmap_uart( 3, UART3_RX_PIN, UART3_TX_PIN, UART3_RTS_PIN, UART3_CTS_PIN );
#endif
#if NUM_UART > 4 && defined( UART4_PINMAP )
  pinmap_uart( 4, UART4_RX_PIN, UART4_TX_PIN, UART4_RTS_PIN, UART4_CTS_PIN );
#endif
#if NUM_UART > 5 && defined( UART5_PINMAP )
  pinmap_uart( 5, UART5_RX_PIN, UART5_TX_PIN, UART5_RTS_PIN, UART5_CTS_PIN );
#endif
#if NUM_UART > 6 && defined( UART6_PINMAP )
  pinmap_uart( 6, UART6_RX_PIN, UART6_TX_PIN, UART6_RTS_PIN, UART6_CTS_PIN );
#endif
#if NUM_UART > 7 && defined( UART7_PINMAP )
  pinmap_uart( 7, UART7_RX_PIN, UART7_TX_PIN, UART7_RTS_PIN, UART7_CTS_PIN );
#endif
#if NUM_UART > 8 && defined( UART8_PINMAP )
  pinmap_uart( 8, UART8_RX_PIN, UART8_TX_PIN, UART8_RTS_PIN, UART8_CTS_PIN );
#endif
#if NUM_UART > 9 && defined( UART9_PINMAP )
  pinmap_uart( 9, UART9_RX_PIN, UART9_TX_PIN, UART9_RTS_PIN, UART9_CTS_PIN );
#endif
#if NUM_UART > 10 && defined( UART10_PINMAP )
  pinmap_uart( 10, UART10_RX_PIN, UART10_TX_PIN, UART10_RTS_PIN, UART10_CTS_PIN );
#endif
#if NUM_UART > 11 && defined( UART11_PINMAP )
  pinmap_uart( 11, UART11_RX_PIN, UART11_TX_PIN, UART11_RTS_PIN, UART11_CTS_PIN );
#endif
#if NUM_UART > 12 && defined( UART12_PINMAP )
  pinmap_uart( 12, UART12_RX_PIN, UART12_TX_PIN, UART12_RTS_PIN, UART12_CTS_PIN );
#endif
#if NUM_UART > 13 && defined( UART13_PINMAP )
  pinmap_uart( 13, UART13_RX_PIN, UART13_TX_PIN, UART13_RTS_PIN, UART13_CTS_PIN );
#endif
#if NUM_UART > 14 && defined( UART14_PINMAP )
  pinmap_uart( 14, UART14_RX_PIN, UART14_TX_PIN, UART14_RTS_PIN, UART14_CTS_PIN );
#endif
#if NUM_UART > 15 && defined( UART15_PINMAP )
  pinmap_uart( 15, UART15_RX_PIN, UART15_TX_PIN, UART15_RTS_PIN, UART15_CTS_PIN );
#endif
  return 1;
}

#else // #ifdef HAS_PINMAPS 

int pinmap_uart( int id, int rx, int tx, int rts, int cts )
{
  return 1;
}

int pinmap_initial_config()
{
  return 1;
}

#endif // #ifdef HAS_PINMAPS 
