// Host I/O operations for eLua 'simulator'

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include "term.h"
#include "host.h"
#include "hostif.h"

#define EOF (-1)
#define STDIN_FILENO 0
#define STDOUT_FILENO 1

void hostif_putc(char c)
{
	host_write(STDOUT_FILENO, &c, 1);
}

void hostif_putstr(const char *c)
{
  int i = 0;
  while( c[ i ] )
    hostif_putc( c[ i++ ] );
}

int hostif_getch()
{
 	unsigned char ch = 0;
	if(host_read(STDIN_FILENO, &ch, 1) != 1) {
		return EOF;
	}
	return (int)ch;
 
}

void* hostif_getmem( unsigned size )
{
  void *pmem = host_mmap2( 0, size, (PROT_READ|PROT_WRITE), (MAP_PRIVATE|MAP_ANONYMOUS), -1, 0 );
  return pmem == MAP_FAILED ? NULL : pmem;
}

void hostif_exit()
{
  host_exit( 0 );
}

int hostif_open( const char* name, int flags, unsigned mode )
{
  return host_open( name, flags, ( mode_t )mode );
}

int hostif_read( int fd, void *buf, unsigned count )
{
  return ( int )host_read( fd, buf, ( size_t )count );
}

int hostif_write( int fd, const void *buf, unsigned count )
{
  return ( int )host_write( fd, buf, ( size_t )count );
}

int hostif_close( int fd )
{
  return host_close( fd );
}

long hostif_lseek( int fd, long pos, int whence )
{
  return host_lseek( fd, pos, whence );
}

s64 hostif_gettime()
{
  struct timeval tv;

  host_gettimeofday( &tv, NULL );
  return ( s64 )tv.tv_sec * 1000000 + tv.tv_usec;
}

