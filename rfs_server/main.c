// Remote FS server

#include "remotefs.h"
#include "serial.h"
#include "server.h"
#include "type.h"
#include "log.h"
#include "os_io.h"
#include <stdio.h>
#include <limits.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

// ****************************************************************************
// Local variables

#define   MAX_PACKET_SIZE     4096

static u8 rfs_buffer[ MAX_PACKET_SIZE + RFS_WRITE_REQUEST_EXTRA ]; 
static ser_handler ser;

// ****************************************************************************
// Helpers

static void flush_serial()
{
  // Flush all data in serial port
  ser_set_timeout_ms( ser, SER_NO_TIMEOUT );
  while( ser_read_byte( ser ) != -1 );
  ser_set_timeout_ms( ser, SER_INF_TIMEOUT );
}

// Read a packet from the serial port
static void read_request_packet()
{
  u16 temp16;
  u32 readbytes;

  while( 1 )
  {
    // First read the length
    if( ( readbytes = ser_read( ser, rfs_buffer, RFS_START_OFFSET ) ) != RFS_START_OFFSET )
    {
      log_msg( "read_request_packet: ERROR reading packet length. Requested %d bytes, got %d bytes\n", RFS_START_OFFSET, readbytes );
      flush_serial();
      continue;
    }

    if( remotefs_get_packet_size( rfs_buffer, &temp16 ) == REMOTEFS_ERR )
    {
      log_msg( "read_request_packet: ERROR getting packet size.\n" );
      flush_serial();
      continue;
    }

    // Then the rest of the data
    if( ( readbytes = ser_read( ser, rfs_buffer + RFS_START_OFFSET, temp16 - RFS_START_OFFSET ) ) != temp16 - RFS_START_OFFSET )
    {
      log_msg( "read_request_packet: ERROR reading full packet, got %u bytes, expected %u bytes\n", ( unsigned )readbytes, ( unsigned )temp16 - RFS_START_OFFSET );
      flush_serial();
      continue;
    }
    else
      break;
  }
}

// Send a packet to the serial port
static void send_response_packet()
{
  u16 temp16;

  // Send request
  if( remotefs_get_packet_size( rfs_buffer, &temp16 ) != REMOTEFS_ERR )
  {
    log_msg( "send_response_packet: sending response packet of %u bytes\n", ( unsigned )temp16 );
    ser_write( ser, rfs_buffer, temp16 );
  }
}

// Secure atoi
static int secure_atoi( const char *str, long *pres )
{
  char *end_ptr;
  long s1;
  
  errno = 0;
  s1 = strtol( str, &end_ptr, 10 );
  if( ( s1 == LONG_MIN || s1 == LONG_MAX ) && errno != 0 )
    return 0;
  else if( end_ptr == str )
    return 0;
  else if( s1 > INT_MAX || s1 < INT_MIN )
    return 0;
  else if( '\0' != *end_ptr )
    return 0;
  *pres = s1;
  return 1;  
}

// ****************************************************************************
// Entry point

#define PORT_ARG_IDX          1
#define SPEED_ARG_IDX         2
#define DIRNAME_ARG_IDX       3
#define VERBOSE_ARG_IDX       4

int main( int argc, const char **argv )
{
  long serspeed;
  
  if( argc < 4 )
  {
    fprintf( stderr, "Usage: %s <port> <speed> <dirname> [-v]\n", argv[ 0 ] );
    fprintf( stderr, "(use -v for verbose output).\n");
    return 1;
  }
  if( secure_atoi( argv[ SPEED_ARG_IDX ], &serspeed ) == 0 )
  {
    fprintf( stderr, "Invalid speed\n" );
    return 1;
  } 
  if( !os_isdir( argv[ DIRNAME_ARG_IDX ] ) )
  {
    fprintf( stderr, "Invalid directory %s\n", argv[ DIRNAME_ARG_IDX ] );
    return 1;
  }
  if( ( argc >= 5 ) && !strcmp( argv[ VERBOSE_ARG_IDX ], "-v" ) )
    log_init( LOG_ALL );
  else
    log_init( LOG_NONE );

  // Setup RFS server
  server_setup( argv[ DIRNAME_ARG_IDX ] );

  // Setup serial port
  if( ( ser = ser_open( argv[ PORT_ARG_IDX ] ) ) == ( ser_handler )-1 )
  {
    fprintf( stderr, "Cannot open port %s\n", argv[ PORT_ARG_IDX ] );
    return 1;
  }
  if( ser_setup( ser, ( u32 )serspeed, SER_DATABITS_8, SER_PARITY_NONE, SER_STOPBITS_1 ) != SER_OK )
  {
    fprintf( stderr, "Unable to initialize serial port\n" );
    return 1;
  }
  flush_serial();
  
  // User report
  printf( "Running RFS server on port %s (%u baud) in directory %s\n", argv[ PORT_ARG_IDX ], ( unsigned )serspeed, argv[ DIRNAME_ARG_IDX ] );  

  // Enter the server endless loop
  while( 1 )
  {
    read_request_packet();
    server_execute_request( rfs_buffer );
    send_response_packet();
  }

  ser_close( ser );
  return 0;
}

