// eLua remote file system support functions

#include "devman.h"
#include "platform_conf.h"
#include "type.h"
#include "platform.h"
#include "remotefs.h"
#include "eluarpc.h"
#include "client.h"
#include "sermux.h"
#include "buf.h"
#include <fcntl.h>
#ifdef ELUA_SIMULATOR
#include "hostif.h"
#endif
#include <stdio.h>

#ifdef BUILD_RFS

// [TODO] the new builder should automatically do this
#ifndef RFS_FLOW_TYPE
#define RFS_FLOW_TYPE         PLATFORM_UART_FLOW_NONE
#endif

#ifndef RFS_TIMER_ID
#define RFS_TIMER_ID          PLATFORM_TIMER_SYS_ID
#endif

// Our RFS buffer
// Compute the usable buffer size starting from RFS_BUFFER_SIZE (which is the
// size of the serial buffer). A complete packet must fit in RFS_BUFFER_SIZE
// bytes. Computed this to be large enough for a WRITE request.
#define RFS_REAL_BUFFER_SIZE      ( ( 1 << RFS_BUFFER_SIZE ) - ELUARPC_WRITE_REQUEST_EXTRA )
static u8 rfs_buffer[ 1 << RFS_BUFFER_SIZE ];

#ifdef ELUA_SIMULATOR
static int rfs_read_fd, rfs_write_fd;
#endif

static int rfs_open_r( struct _reent *r, const char *path, int flags, int mode, void *pdata )
{
  return rfsc_open( path, flags, mode );
}

static int rfs_close_r( struct _reent *r, int fd, void *pdata )
{
  return rfsc_close( fd );
}

static _ssize_t rfs_write_r( struct _reent *r, int fd, const void* ptr, size_t len, void *pdata )
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

static _ssize_t rfs_read_r( struct _reent *r, int fd, void* ptr, size_t len, void *pdata )
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
static off_t rfs_lseek_r( struct _reent *r, int fd, off_t off, int whence, void *pdata )
{
  return ( off_t )rfsc_lseek( fd, ( s32 )off, whence );
}

// opendir
static void* rfs_opendir_r( struct _reent *r, const char* name, void *pdata )
{
  return ( void* )rfsc_opendir( name );
}

// readdir
static struct dm_dirent* rfs_readdir_r( struct _reent *r, void *d, void *pdata )
{
  static struct dm_dirent ent;

  rfsc_readdir( ( u32 )d, &ent.fname, &ent.fsize, &ent.ftime );
  if( ent.fname == NULL )
    return NULL;
  return &ent;
}

// closedir
static int rfs_closedir_r( struct _reent *r, void *d, void *pdata )
{
  return rfsc_closedir( ( u32 )d );
}

// ****************************************************************************
// Remote FS serial transport functions

#ifdef RFS_UART_ID
static u32 rfs_send( const u8 *p, u32 size )
{
  unsigned i;

  for( i = 0; i < size; i ++ )
    platform_uart_send( RFS_UART_ID, p[ i ] );
  return size;
}

static u32 rfs_recv( u8 *p, u32 size, timer_data_type timeout )
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
#endif

// ****************************************************************************
// Remote FS pipe transport functions (used only in simulator)

#ifdef ELUA_CPU_LINUX
static u32 rfs_send( const u8 *p, u32 size )
{
  return ( u32 )hostif_write( rfs_write_fd, p, size );
}

static u32 rfs_recv( u8 *p, u32 size, timer_data_type timeout )
{
  timeout = timeout;
  return ( u32 )hostif_read( rfs_read_fd, p, size );
}
#endif

// Our remote file system device descriptor structure
static const DM_DEVICE rfs_device = 
{
  rfs_open_r,           // open
  rfs_close_r,          // close
  rfs_write_r,          // write
  rfs_read_r,           // read
  rfs_lseek_r,          // lseek
  rfs_opendir_r,        // opendir
  rfs_readdir_r,        // readdir
  rfs_closedir_r,       // closedir
  NULL                  // getaddr
};

int remotefs_init()
{
#ifdef ELUA_CPU_LINUX 
  // Open our read/write pipes
  rfs_read_fd = hostif_open( RFS_SRV_WRITE_PIPE, O_RDONLY, 0 );
  rfs_write_fd = hostif_open( RFS_SRV_READ_PIPE, O_WRONLY, 0 );
  if( rfs_read_fd == -1 || rfs_write_fd == -1 )
  {
    hostif_putstr( "unable to open read/write pipes\n" );
    return DM_ERR_INIT;
  }
#elif RFS_UART_ID < SERMUX_SERVICE_ID_FIRST  // if RFS runs on a virtual UART, buffers are already set in common.c
  // Initialize RFS UART
  platform_uart_setup( RFS_UART_ID, RFS_UART_SPEED, 8, PLATFORM_UART_PARITY_NONE, PLATFORM_UART_STOPBITS_1 );
  platform_uart_set_flow_control( RFS_UART_ID, RFS_FLOW_TYPE );
  if( platform_uart_set_buffer( RFS_UART_ID, RFS_BUFFER_SIZE ) == PLATFORM_ERR )
  {
    printf( "WARNING: unable to initialize RFS filesystem\n" );
    return DM_ERR_INIT;
  } 
#endif
  rfsc_setup( rfs_buffer, rfs_send, rfs_recv, RFS_TIMEOUT );
  return dm_register( "/rfs", NULL, &rfs_device );
}

#else // #ifdef BUILD_RFS

int remotefs_init()
{
  return dm_register( NULL, NULL, NULL );
}

#endif

