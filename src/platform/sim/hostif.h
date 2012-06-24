// Host I/O operations for eLua 'simulator'

#ifndef __HOSTIO_H__
#define __HOSTIO_H__

#include "type.h"

// Write a single character out to the screen.
void hostif_putc(char c);

// Output a null-terminated ASCII string to the monitor.
void hostif_putstr(const char *c);

// Get a char from keyboard
int hostif_getch();

// Get memory
void *hostif_getmem( unsigned size );

// Terminate the simulator (exit program)
void hostif_exit();

// Open
int hostif_open( const char* name, int flags, unsigned mode );

// Read
int hostif_read( int fd, void *buf, unsigned count );

// Write
int hostif_write( int fd, const void *buf, unsigned count );

// Close
int hostif_close( int fd );

// Seek
long hostif_lseek( int fd, long pos, int whence );

// Get time
s64 hostif_gettime();

#endif // __HOSTIO_H__

