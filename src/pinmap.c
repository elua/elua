// Pin mapping functions

#include "platform_conf.h"
#ifdef HAS_PINMAPS 

#define IMPLEMENT_PINMAPS
#include "platform.h"
#include "pinmap.h"
#include ELUA_CPU_PINMAP_HEADER

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
      return platform_pio_has_pin( PLATFORM_IO_GET_PORT( pinfo->pin ), PLATFORM_IO_GET_PIN( pinfo->pin ) ) ? pinfo->pfuncs : NULL;
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

static int pinmaph_create_mapping( int peripheral_id, int id, int num_pins, const int* ppins )
{
  pin_be_info pin_info[ num_pins ];
  int i, temp;
  const pin_function* pf;

  for( i = 0; i < num_pins; i ++ )
  {
    if( ppins[ i ] == IGNORE_PIN )
      continue;
    if( ( pf = pinmaph_find_pin( ppins[ i ] ) ) == NULL )
      return 0;
    if( ( temp = pinmaph_find_function( pf, peripheral_id, id, i ) ) == -1 )
      return 0;
    pin_info[ i ] = ( pin_be_info )temp;
  }
  for( i = 0; i < num_pins; i ++ )
    if( ppins[ i ] != IGNORE_PIN )
      platform_pio_set_function( PLATFORM_IO_GET_PORT( ppins[ i ] ), PLATFORM_IO_GET_PIN( ppins[ i ] ), pin_info[ i ] );
  return 1;
}

// Initial pinmap configurations
static int pinmaph_initial_config( int id, int num_pins, const int* ppins )
{
  int i = 0;

  while( ppins[ i ] != -1 )
  {
    pinmaph_create_mapping( id, ppins[ i ], num_pins, ppins + i + 1 );
    i += num_pins + 1;
  }
  return 1;
}

////////////////////////////////////////////////////////////////////////////////
// Public interface

int pinmap_uart( int id, int rx, int tx, int rts, int cts )
{
  int pins[ 4 ] = { rx, tx, rts, cts };

  if( id >= NUM_UART )
    return 0;
  return pinmaph_create_mapping( PINMAP_UART, id, 4, pins );
}

int pinmap_initial_config()
{
#ifdef INITIAL_UART_PINMAPS
  {
    int uart_initial_maps[] = INITIAL_UART_PINMAPS;
    pinmaph_initial_config( PINMAP_UART, 4, uart_initial_maps );
  }
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
