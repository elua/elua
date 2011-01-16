// Remote FS server

#include "remotefs.h"
#include "server.h"
#include "type.h"
#include "log.h"
#include "os_io.h"
#include <stdio.h>
#include <limits.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

// ****************************************************************************
// Local variables

#define   MAX_PACKET_SIZE     4096

static u8 rfs_buffer[ MAX_PACKET_SIZE + RFS_WRITE_REQUEST_EXTRA ]; 
static int rfs_read_fd;
static int rfs_write_fd;

// ****************************************************************************
// Helpers

// Read a packet from the serial port
static void read_request_packet()
{
  u16 temp16;
  u32 readbytes;

  while( 1 )
  {
    // First read the length
    if( ( readbytes = read( rfs_read_fd, rfs_buffer, RFS_START_OFFSET ) ) != RFS_START_OFFSET )
    {
//      log_msg( "read_request_packet: ERROR reading packet length. Requested %d bytes, got %d bytes\n", RFS_START_OFFSET, readbytes );
      continue;
    }

    if( remotefs_get_packet_size( rfs_buffer, &temp16 ) == REMOTEFS_ERR )
    {
      // log_msg( "read_request_packet: ERROR getting packet size.\n" );
      continue;
    }

    // Then the rest of the data
    if( ( readbytes = read( rfs_read_fd, rfs_buffer + RFS_START_OFFSET, temp16 - RFS_START_OFFSET ) ) != temp16 - RFS_START_OFFSET )
    {
      // log_msg( "read_request_packet: ERROR reading full packet, got %u bytes, expected %u bytes\n", ( unsigned )readbytes, ( unsigned )temp16 - RFS_START_OFFSET );
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
    write( rfs_write_fd, rfs_buffer, temp16 );
  }
}

// ****************************************************************************
// Entry point

#define DIRNAME_ARG_IDX       1
#define VERBOSE_ARG_IDX       2

int main( int argc, const char **argv )
{
  if( argc < 2 )
  {
    fprintf( stderr, "Usage: %s <dirname> [-v]\n", argv[ 0 ] );
    fprintf( stderr, "(use -v for verbose output).\n");
    return 1;
  }
  if( !os_isdir( argv[ DIRNAME_ARG_IDX ] ) )
  {
    fprintf( stderr, "Invalid directory %s\n", argv[ DIRNAME_ARG_IDX ] );
    return 1;
  }
  if( ( argc >= 3 ) && !strcmp( argv[ VERBOSE_ARG_IDX ], "-v" ) )
    log_init( LOG_ALL );
  else
    log_init( LOG_NONE );

  // Create and open FIFOs
  mkfifo( RFS_SRV_READ_PIPE, 0666 );
  mkfifo( RFS_SRV_WRITE_PIPE, 0666 );
  rfs_write_fd = open( RFS_SRV_WRITE_PIPE, O_WRONLY, 0 );
  rfs_read_fd = open( RFS_SRV_READ_PIPE, O_RDONLY, 0 );
  if( rfs_read_fd == -1 || rfs_write_fd == -1 )
  {
    fprintf( stderr, "Unable to open pipes\n" );
    return 1;
  }
  printf( "Running in SIM mode (pipes)\n" );

  // Setup RFS server
  server_setup( argv[ DIRNAME_ARG_IDX ] );

  // Enter the server endless loop
  while( 1 )
  {
    read_request_packet();
    server_execute_request( rfs_buffer );
    send_response_packet();
  }

  close( rfs_write_fd );
  close( rfs_read_fd );
  unlink( RFS_SRV_READ_PIPE );
  unlink( RFS_SRV_WRITE_PIPE );
  return 0;
}

