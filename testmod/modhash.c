// Hash a file, append hash in the beginning

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* UNIX ELF hash
 * Published hash algorithm used in the UNIX ELF format for object files */

unsigned int hash( const char* str, unsigned int len )
{
  unsigned int hash = 0;
  unsigned int x    = 0;
  unsigned int i    = 0;

  for( i = 0; i < len; str ++, i ++ )
  {
    hash = ( hash << 4 ) + ( *str );
    if( ( x = hash & 0xF0000000L ) != 0 )
      hash ^= ( x >> 24 );
    hash &= ~x;
  }
  return hash;
}

int main( int argc, char **argv )
{
  FILE *fp;
  char *pdata;
  long size;
  unsigned int fhash;
  int be = 0;

  if( argc < 2 )
  {
    fprintf( stderr, "Usage: %s <filename> [-b]\n", argv[ 0 ] );
    fprintf( stderr, "Use '-b' for big endian mode\n" );
    return 1;
  }
  if( argc >= 3 && !strcmp( argv[ 2 ], "-b" ) )
  {
    printf( "Using big endian mode\n" );
    be = 1;
  }
  if( ( fp = fopen( argv[ 1 ], "rb" ) ) == NULL )
  {
    fprintf( stderr, "Unable to open %s\n", argv[ 1 ] );
    return 1;
  }
  fseek( fp, 0, SEEK_END );
  size = ftell( fp );
  fseek( fp, 0, SEEK_SET );
  if( ( pdata = ( char* )malloc( size ) ) == NULL )
  {
    fprintf( stderr, "Unable to allocate memor\n" );
    return 1;
  }
  fread( pdata, 1, size, fp );
  fclose( fp );
  fhash = hash( pdata, size );
  printf( "Computed hash is %08X\n", fhash );
  if( be )
    fhash = ( fhash >> 24 ) | ( ( fhash << 8 ) & 0x00FF0000 ) |  ( ( fhash >> 8 ) & 0x0000FF00 ) | ( fhash << 24 );
  if( ( fp = fopen( argv[ 1 ], "wb" ) ) == NULL )
  {
    fprintf( stderr, "Cannot open %s in write mode.\n", argv[ 1 ] );
    return 1;
  }
  fwrite( &fhash, 1, 4, fp );
  fwrite( pdata, 1, size, fp );
  fclose( fp );
  printf( "Hash written back to file\n" );
  return 0;
}

