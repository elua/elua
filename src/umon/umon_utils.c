// Various utility functions for the monitor

#ifdef BUILD_UMON

#include "umon.h"
#include "umon_utils.h"
#include <stdarg.h>

// ****************************************************************************
// Local functions

static void umonh_print_10u( unsigned n )
{
  char c[ 20 ];
  int ndig = 0;

  if( n == 0 )
  {
    umon_putc( '0' );
    return;
  }
  while( n )
  {
    c[ ndig ++ ] = n % 10;
    n = n / 10;
  }
  while( ndig > 0 )
    umon_putc( c[ --ndig ] );
}

static void umonh_print_10s( int n )
{
  unsigned u = ( unsigned )n;

  if( n < 0 )
  {
    umon_putc( '-' );
    u = -n;
  }
  umonh_print_10u( u );
}

static void umonh_print_hex4( unsigned n )
{
  umon_putc( n < 10 ? n + '0' : n - 10 + 'a' );
}

static void umonh_print_hex8( unsigned n )
{
  umonh_print_hex4( ( n >> 4 ) & 0x0F );
  umonh_print_hex4( n & 0x0F );
}

static void umonh_print_hex16( unsigned n )
{
  umonh_print_hex8( ( n >> 8 ) & 0xFF );
  umonh_print_hex8( n & 0xFF );
}

static void umonh_print_hex32( unsigned n )
{
  umonh_print_hex16( ( n >> 16 ) & 0xFFFF );
  umonh_print_hex16( n & 0xFFFF );
}

// ****************************************************************************
// Public functions

void umon_printf( const char *fmt, ... )
{
  va_list ap;
  const char *p;

  va_start( ap, fmt );
  while( *fmt )
  {
    if( *fmt == '%' )
    {
      fmt ++;
      switch( *fmt )
      {
        case '%':
          umon_putc( '%' );
          break;

        case 'c':
          umon_putc( ( char )va_arg( ap, int ) );
          break;

        case 's':
          p = va_arg( ap, const char* );
          while( *p )
            umon_putc( *p ++ );
          break;

        case 'i':
        case 'd':
          umonh_print_10s( va_arg( ap, int ) );
          break;

        case 'u':
          umonh_print_10u( va_arg( ap, unsigned ) );
          break;

        case 'x':
        case 'X':
          umonh_print_hex32( va_arg( ap, unsigned ) );
          break;

        case 'p':
          umonh_print_hex32( ( unsigned )va_arg( ap, void* ) );
          break;
      }
    }
    else
      umon_putc( *fmt );
    fmt ++;
  }
  va_end( ap );
}

#endif // #ifdef BUILD_UMON

