// Generic serial interface

#ifndef __SERIAL_H__
#define __SERIAL_H__

#include "type.h"

#define SER_INF_TIMEOUT         0xFFFFFFFF
#define SER_NO_TIMEOUT          0
#define SER_OK                  0
#define SER_ERR                 1

// Serial interface modes (blocking or non blocking)
#define SER_MODE_BLOCKING       0
#define SER_MODE_NONBLOCKING    1

// Setup constants
#define SER_PARITY_NONE         0
#define SER_PARITY_EVEN         1
#define SER_PARITY_ODD          2

#define SER_STOPBITS_1          0
#define SER_STOPBITS_1_5        1
#define SER_STOPBITS_2          2

#define SER_DATABITS_5          5
#define SER_DATABITS_6          6
#define SER_DATABITS_7          7
#define SER_DATABITS_8          8

// Define serial port "handle" type for each platform
#ifdef WIN32_BUILD
#include <windows.h>
typedef HANDLE ser_handler;
#else // assume POSIX here
typedef int ser_handler;
#endif

// Serial access functions (to be implemented by each platform)
ser_handler ser_open( const char *sername );
void ser_close( ser_handler id );
int ser_setup( ser_handler id, u32 baud, int databits, int parity, int stopbits );
u32 ser_read( ser_handler id, u8* dest, u32 maxsize );
int ser_read_byte( ser_handler id );
u32 ser_write( ser_handler id, const u8 *src, u32 size );
u32 ser_write_byte( ser_handler id, u8 data );
void ser_set_timeout_ms( ser_handler id, u32 timeout );
int ser_readable( ser_handler id );

#endif
