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
  int i;

  if( pin == PINMAP_IGNORE_PIN )
    return NULL;
  for( i = 0; i < sizeof( pinfuncs ) / sizeof( pin_info ); i ++ )
    if( pinfuncs[ i ].pin == ( u16 )pin )
      return platform_pio_has_pin( PLATFORM_IO_GET_PORT( pinfuncs[ i ].pin ), PLATFORM_IO_GET_PIN( pinfuncs[ i ].pin ) ) ? pinfuncs[ i ].pfuncs : NULL;
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
    if( ppins[ i ] == PINMAP_IGNORE_PIN )
      continue;
    if( ( pf = pinmaph_find_pin( ppins[ i ] ) ) == NULL )
      return 0;
    if( ( temp = pinmaph_find_function( pf, peripheral_id, id, i ) ) == -1 )
      return 0;
    pin_info[ i ] = ( pin_be_info )temp;
  }
  for( i = 0; i < num_pins; i ++ )
    if( ppins[ i ] != PINMAP_IGNORE_PIN )
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
  int pins[ PINMAP_UART_TOTAL ] = { rx, tx, rts, cts };

  if( id >= NUM_UART )
    return 0;
  return pinmaph_create_mapping( PINMAP_UART, id, PINMAP_UART_TOTAL, pins );
}

int pinmap_spi( int id, int mosi, int miso, int sck )
{
  int pins[ PINMAP_SPI_TOTAL ] = { mosi, miso, sck };

  if( id >= NUM_SPI )
    return 0;
  return pinmaph_create_mapping( PINMAP_SPI, id, PINMAP_SPI_TOTAL, pins );
}

int pinmap_initial_config()
{
#ifdef INITIAL_UART_PINMAPS
  {
    int uart_initial_maps[] = INITIAL_UART_PINMAPS;
    pinmaph_initial_config( PINMAP_UART, PINMAP_UART_TOTAL, uart_initial_maps );
  }
#endif
#ifdef INITIAL_SPI_PINMAPS
  {
    int spi_initial_maps[] = INITIAL_SPI_PINMAPS;
    pinmaph_initial_config( PINMAP_SPI, PINMAP_SPI_TOTAL, spi_initial_maps );
  }
#endif
  return 1;
}

int pinmap_get_num_pins()
{
  return sizeof( pinfuncs ) / sizeof( pin_info );
}

const pin_info* pinmap_get_at( int i )
{
  if( i < 0 || i > sizeof( pinfuncs ) / sizeof( pin_info ) )
    return NULL;
  return pinfuncs + i;
}

#else // #ifdef HAS_PINMAPS 

int pinmap_uart( int id, int rx, int tx, int rts, int cts )
{
  return 0;
}

int pinmap_spi( int id, int mosi, int miso, int sck )
{
  return 0;
}

int pinmap_initial_config()
{
  return 0;
}

#endif // #ifdef HAS_PINMAPS 
