// Host I/O operations for eLua 'simulator'

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include "term.h"
#include "host.h"
#include "hostif.h"

#define EOF (-1)
#define STDIN_FILENO 0
#define STDOUT_FILENO 1

void hostif_put(char c)
{
	host_write(STDOUT_FILENO, &c, 1);
}

void hostif_write(const char *c)
{
  int i = 0;
  while (c[i])
    hostif_put(c[i++]);
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

