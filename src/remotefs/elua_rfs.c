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
#include "elua_net.h"
#include "utils.h"
#include <fcntl.h>
#include <string.h>
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

// opendir
static void* rfs_opendir_r( struct _reent *r, const char* name )
{
  return ( void* )rfsc_opendir( name );
}

// readdir
static struct dm_dirent* rfs_readdir_r( struct _reent *r, void *d )
{
  static struct dm_dirent ent;

  rfsc_readdir( ( u32 )d, &ent.fname, &ent.fsize, &ent.ftime );
  if( ent.fname == NULL )
    return NULL;
  return &ent;
}

// closedir
static int rfs_closedir_r( struct _reent *r, void *d )
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
// UDP transport implementation

#ifdef RFS_TRANSPORT_UDP
static int rfs_socket = ELUA_NET_INVALID_SOCKET;
static volatile elua_net_ip rfs_server_ip;
static volatile u16 rfs_data_size;
static p_elua_net_state_cb rfs_prev_state_cb;

#define RFS_MAX_DISCOVERIES   3
#define RFS_DISCOVERY_TO      40000

// Receive callback (directly from the TCP stack)
static void rfs_recv_cb( int sockno, const u8 *pdata, unsigned size, elua_net_ip ip, u16 port )
{
  if( rfs_socket == ELUA_NET_INVALID_SOCKET )
    return;
  ( void )sockno;
  if( rfs_server_ip.ipaddr == 0 && size == ELUARPC_START_OFFSET && eluarpc_is_discover_response_packet( pdata ) ) // this is a response for the discovery request
  {
    rfs_server_ip.ipaddr = ip.ipaddr;
    rfs_data_size = size;
  }
  else // regular data packet
  {
    rfs_data_size = UMIN( size, 1 << RFS_BUFFER_SIZE );
    memcpy( rfs_buffer, pdata, rfs_data_size );
  }
}

// Check if the client is initialized, send a request for the server
// if it is not
static int rfs_lookup_server()
{
  elua_net_ip ip;
  timer_data_type tmrstart = 0;
  int retries = 0;
  u8 discover_packet[ ELUARPC_START_OFFSET ];

  if( rfs_socket == ELUA_NET_INVALID_SOCKET )
    return 0;
  if( rfs_server_ip.ipaddr != 0 )
    return 1;
  ip.ipaddr = 0xFFFFFFFF;
  eluarpc_build_discover_packet( discover_packet );
  while( rfs_server_ip.ipaddr == 0 && retries < RFS_MAX_DISCOVERIES )
  {
    elua_net_sendto( rfs_socket, discover_packet, ELUARPC_START_OFFSET, ip, RFS_UDP_PORT );
    tmrstart = platform_timer_op( RFS_TIMER_ID, PLATFORM_TIMER_OP_START, 0 );
    while( 1 )
    {
      if( rfs_data_size )
        break;
      if( platform_timer_get_diff_crt( RFS_TIMER_ID, tmrstart ) >= RFS_DISCOVERY_TO )
        break;
    }
    rfs_data_size = 0;
    retries ++;
  }
  return rfs_server_ip.ipaddr != 0;
}

static u32 rfs_send( const u8 *p, u32 size )
{
  if( rfs_socket == ELUA_NET_INVALID_SOCKET )
    return 0;
  if( !rfs_lookup_server() )
    return 0;
  return elua_net_sendto( rfs_socket, p, size, rfs_server_ip, RFS_UDP_PORT );
}

static u32 rfs_recv( u8 *p, u32 size, timer_data_type timeout )
{
  u32 readbytes = 0;
  timer_data_type tmrstart = 0;

  ( void )p;
  ( void )size;
  if( rfs_socket == ELUA_NET_INVALID_SOCKET )
    return 0;
  if( rfs_server_ip.ipaddr == 0 ) // this shouldn't happen at all
    return 0;
  if( timeout > 0 )
    tmrstart = platform_timer_op( RFS_TIMER_ID, PLATFORM_TIMER_OP_START, 0 );
  while( 1 )
  {
    if( rfs_data_size )
      break;
    if( timeout == 0 || ( timeout != PLATFORM_TIMER_INF_TIMEOUT && platform_timer_get_diff_crt( RFS_TIMER_ID, tmrstart ) >= timeout ) )
      break;
  }
  readbytes = rfs_data_size;
  rfs_data_size = 0;
  if( readbytes == 0 ) // server error, must search again
    rfs_server_ip.ipaddr = 0;
  return readbytes;
}

static void rfs_state_cb( int state )
{
  if( state == ELUA_NET_STATE_DOWN )
    rfs_socket = ELUA_NET_INVALID_SOCKET;
  else
  {
    if( ( rfs_socket = elua_net_socket( ELUA_NET_SOCK_DGRAM ) ) != ELUA_NET_INVALID_SOCKET )
      elua_net_set_socket_callback( rfs_socket, rfs_recv_cb );
  }
  if( rfs_prev_state_cb )
    rfs_prev_state_cb( state );
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
  "/rfs",
  rfs_open_r,           // open
  rfs_close_r,          // close
  rfs_write_r,          // write
  rfs_read_r,           // read
  rfs_lseek_r,          // lseek
  rfs_opendir_r,        // opendir
  rfs_readdir_r,        // readdir
  rfs_closedir_r        // closedir
};

const DM_DEVICE *remotefs_init()
{
#ifdef ELUA_CPU_LINUX 
  // Open our read/write pipes
  rfs_read_fd = hostif_open( RFS_SRV_WRITE_PIPE, O_RDONLY, 0 );
  rfs_write_fd = hostif_open( RFS_SRV_READ_PIPE, O_WRONLY, 0 );
  if( rfs_read_fd == -1 || rfs_write_fd == -1 )
  {
    hostif_putstr( "unable to open read/write pipes\n" );
    return NULL;
  }
#elif defined( RFS_UART_ID ) && RFS_UART_ID < SERMUX_SERVICE_ID_FIRST  // if RFS runs on a virtual UART, buffers are already set in common.c
  // Initialize RFS UART
  platform_uart_setup( RFS_UART_ID, RFS_UART_SPEED, 8, PLATFORM_UART_PARITY_NONE, PLATFORM_UART_STOPBITS_1 );
  platform_uart_set_flow_control( RFS_UART_ID, RFS_FLOW_TYPE );
  if( platform_uart_set_buffer( RFS_UART_ID, RFS_BUFFER_SIZE ) == PLATFORM_ERR )
  {
    printf( "WARNING: unable to initialize RFS filesystem\n" );
    return NULL;
  } 
#elif defined( RFS_TRANSPORT_UDP ) // goodie!
  if( ( rfs_socket = elua_net_socket( ELUA_NET_SOCK_DGRAM ) ) != ELUA_NET_INVALID_SOCKET )
    elua_net_set_socket_callback( rfs_socket, rfs_recv_cb );
  rfs_prev_state_cb = elua_net_set_state_cb( rfs_state_cb );
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

