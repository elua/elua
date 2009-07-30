// Host I/O operations for eLua 'simulator'

#ifndef __HOSTIO_H__
#define __HOSTIO_H__

// Write a single character out to the screen.
void hostif_put(char c);

// Output a null-terminated ASCII string to the monitor.
void hostif_write(const char *c);

// Get a char from keyboard
int hostif_getch();

// Get memory
void *hostif_getmem( unsigned size );

// Terminate the simulator (exit program)
void hostif_exit();

#endif // __HOSTIO_H__

