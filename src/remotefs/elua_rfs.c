// eLua remote file system support functions

#include "devman.h"
#include "platform_conf.h"
#include "type.h"
#include "platform.h"
#include "remotefs.h"
#include "client.h"
#include "buf.h"
#include <stdio.h>

#ifdef BUILD_RFS

// Our RFS buffer
// Compute the usable buffer size starting from RFS_BUFFER_SIZE (which is the
// size of the serial buffer). A complete packet must fit in RFS_BUFFER_SIZE
// bytes. Computed this to be large enough for a WRITE request.
#define RFS_REAL_BUFFER_SIZE      ( ( 1 << RFS_BUFFER_SIZE ) - RFS_WRITE_REQUEST_EXTRA )
static u8 rfs_buffer[ 1 << RFS_BUFFER_SIZE ];

static int rfs_open_r( struct _reent *r, const char *path, int flags, int mode )
{
  return rfsc_open( path, flags, mode );
}

static int rfs_close_r( struct _reent *r, int fd )
{
  return rfsc_close( fd );
}

static _ssize_t rfs_write_r( struct _reent *r, int fd, const void* ptr, size_t len )
{ 
  s32 total = 0, res;
  u32 towrite;
  const u8 *p = ( const u8* )ptr;

  // Write in RFS_REAL_BUFFER_SIZE increments
//  printf( "Got WRITE request for %d bytes\n", len );
  while( len )
  {
    towrite = len > RFS_REAL_BUFFER_SIZE ? RFS_REAL_BUFFER_SIZE : len;
    if( ( res = rfsc_write( fd, p, towrite ) ) == -1 )
      break;
    total += res;
//    printf( "Wrote %d bytes\n", res );
    if( res < towrite )
      break;
    len -= towrite;
    p += towrite; 
  }
  return ( _ssize_t )total;
}

static _ssize_t rfs_read_r( struct _reent *r, int fd, void* ptr, size_t len )
{
  s32 total = 0, res;
  u32 toread;
  u8 *p = ( u8* )ptr;

  // Read in RFS_REAL_BUFFER_SIZE increments
//  printf( "Got READ request for %d bytes\n", len );
  while( len )
  {
    toread = len > RFS_REAL_BUFFER_SIZE ? RFS_REAL_BUFFER_SIZE : len;
    if( ( res = rfsc_read( fd, p, toread ) ) == -1 )
      break;
    total += res; 
//    printf( "Read %d bytes\n", res );
    if( res < toread )
      break;
    len -= toread;
    p += toread;
  }
  return ( _ssize_t )total;
}

// lseek
static off_t rfs_lseek_r( struct _reent *r, int fd, off_t off, int whence )
{
  return ( off_t )rfsc_lseek( fd, ( s32 )off, whence );
}

// ****************************************************************************
// Remote FS serial transport functions

static u32 rfs_send( const u8 *p, u32 size )
{
  unsigned i;

  for( i = 0; i < size; i ++ )
    platform_uart_send( RFS_UART_ID, p[ i ] );
  return size;
}

static u32 rfs_recv( u8 *p, u32 size, u32 timeout )
{
  u32 cnt = 0;
  int data;

  while( size )
  {
    if( ( data = platform_uart_recv( RFS_UART_ID, RFS_TIMER_ID, timeout ) ) == -1 )
      break;
    *p ++ = ( u8 )data;
    cnt ++;
    size --;
  }
  return cnt;
}

// Our remote file system device descriptor structure
static const DM_DEVICE rfs_device = 
{
  "/rfs",
  rfs_open_r,  
  rfs_close_r, 
  rfs_write_r,
  rfs_read_r,
  rfs_lseek_r,
  NULL
};

const DM_DEVICE *remotefs_init()
{
#ifdef RFS_UART_SPEED
  // Initialize RFS UART
  platform_uart_setup( RFS_UART_ID, RFS_UART_SPEED, 8, PLATFORM_UART_PARITY_NONE, PLATFORM_UART_STOPBITS_1 );
  // [TODO] this isn't exactly right
  buf_set( BUF_ID_UART, RFS_UART_ID, RFS_BUFFER_SIZE, BUF_DSIZE_U8 ); 
#endif
  rfsc_setup( rfs_buffer, rfs_send, rfs_recv, RFS_TIMEOUT );
  return &rfs_device;
}

#else // #ifdef BUILD_RFS

const DM_DEVICE *remotefs_init()
{
  return NULL;
}

#endif

