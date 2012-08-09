// Various utility functions for the monitor

#ifdef BUILD_UMON

#include "umon.h"
#include "umon_utils.h"
#include "compilerdefs.h"
#include <stdarg.h>

// ****************************************************************************
// Local functions

#define STRCATC( s, c ) s[ umon_strlen( s ) ] = ( c )

static void umonh_print_10u( unsigned, int ) NO_INSTRUMENT;
static void umonh_print_10u( unsigned n, int totdigs )
{
  char c[ 20 ];
  int ndig = 0;

  if( n == 0 )
  {
    while( totdigs-- )
      umon_putc( '0' );
    return;
  }
  while( n )
  {
    c[ ndig ++ ] = n % 10;
    n = n / 10;
  }
  while( totdigs-- > ndig )
    umon_putc( '0' );
  while( ndig > 0 )
    umon_putc( c[ --ndig ] );
}

static void umonh_print_10s( int, int ) NO_INSTRUMENT;
static void umonh_print_10s( int n, int totdigs )
{
  unsigned u = ( unsigned )n;

  if( n < 0 )
  {
    umon_putc( '-' );
    u = -n;
  }
  umonh_print_10u( u, totdigs );
}

static void umonh_print_hex4( unsigned, char * ) NO_INSTRUMENT;
static void umonh_print_hex4( unsigned n, char *s )
{
  STRCATC( s, n < 10 ? n + '0' : n - 10 + 'a' );
}

static void umonh_print_hex8( unsigned, char * ) NO_INSTRUMENT;
static void umonh_print_hex8( unsigned n, char *s )
{
  umonh_print_hex4( ( n >> 4 ) & 0x0F, s );
  umonh_print_hex4( n & 0x0F, s );
}

static void umonh_print_hex16( unsigned, char * ) NO_INSTRUMENT;
static void umonh_print_hex16( unsigned n, char *s )
{
  umonh_print_hex8( ( n >> 8 ) & 0xFF, s );
  umonh_print_hex8( n & 0xFF, s );
}

static void umonh_print_hex32( unsigned , int ) NO_INSTRUMENT;
static void umonh_print_hex32( unsigned n, int totdigs )
{  
  char num[ 9 ];
  int i, start = 0;

  for( i = 0; i < 9; i ++ )
    num[ i ] = 0;
  umonh_print_hex16( ( n >> 16 ) & 0xFFFF, num );
  umonh_print_hex16( n & 0xFFFF, num );
  while( num[ start ++ ] == '0' );
  start --;
  if( !num[ start ] )
    start --;
  if( totdigs > 8 - start )
    start -= totdigs - 8 + start;
  while( num[ start ] )
    umon_putc( num[ start ++ ] );
}

// ****************************************************************************
// Public functions

void umon_printf( const char *fmt, ... )
{
  va_list ap;
  const char *p;
  int ndigs = -1;

  va_start( ap, fmt );
  while( *fmt )
  {
    ndigs = -1;
    if( *fmt == '%' )
    {
      fmt ++;
      if( ( '0' <= *fmt ) && ( *fmt <= '9' ) )
        ndigs = *fmt ++ - '0';
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
          umonh_print_10s( va_arg( ap, int ), ndigs );
          break;

        case 'u':
          umonh_print_10u( va_arg( ap, unsigned ), ndigs );
          break;

        case 'x':
        case 'X':
          umonh_print_hex32( va_arg( ap, unsigned ), ndigs );
          break;

        case 'p':
          umonh_print_hex32( ( unsigned )va_arg( ap, void* ), ndigs );
          break;
      }
    }
    else
    {
      if( *fmt == '\n' )
        umon_putc( '\r' );
      umon_putc( *fmt );
    }
    fmt ++;
  }
  va_end( ap );
}

int umon_strlen( const char *s )
{
  int len = 0;

  while( *s ++ )
    len ++;
  return len;
}

#endif // #ifdef BUILD_UMON

