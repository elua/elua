// Remote filesystem client

#include <string.h>
#include "remotefs.h"
#include "client.h"
#include "os_io.h"
#include "eluarpc.h"
#include "platform.h"
#include <stdio.h>
#include "platform_conf.h"
#include "buf.h"

#ifdef BUILD_RFS

#if 0
#define RFSDEBUG        printf
#else
void RFSDEBUG( const char* dummy, ... )
{
}
#endif

// ****************************************************************************
// Client local data

static u8 *rfsc_buffer;
static p_rfsc_send rfsc_send;
static p_rfsc_recv rfsc_recv;
static timer_data_type rfsc_timeout;

// ****************************************************************************
// Client helpers

static int rfsch_send_request_read_response()
{
  u16 temp16;
  u32 readbytes;

#ifndef ELUA_CPU_LINUX
  // Empty receive buffer
  while( rfsc_recv( rfsc_buffer, 1, 0 ) == 1 );
#endif

  // Send request
  if( eluarpc_get_packet_size( rfsc_buffer, &temp16 ) == ELUARPC_ERR )
  {
    RFSDEBUG( "[RFS] get packet size error\n" );
    return CLIENT_ERR;
  }
  if( rfsc_send( rfsc_buffer, temp16 ) != temp16 )
  {
    RFSDEBUG( "[RFS] rfsc_send error\n" );
    return CLIENT_ERR;
  }
  
  // Get response
  // First the length, then the rest of the data
  if( ( readbytes = rfsc_recv( rfsc_buffer, ELUARPC_START_OFFSET, rfsc_timeout ) ) != ELUARPC_START_OFFSET )
  {
    RFSDEBUG( "[RFS] rfsc_recv (1) error: expected %u, got %u\n", ( unsigned )( temp16 - ELUARPC_START_OFFSET ), ( unsigned )readbytes );
    return CLIENT_ERR;
  }
  if( eluarpc_get_packet_size( rfsc_buffer, &temp16 ) == ELUARPC_ERR )
  {
    RFSDEBUG( "[RFS] eluarpc_get_packet_size() error\n" );
    return CLIENT_ERR;
  }
  if( ( readbytes = rfsc_recv( rfsc_buffer + ELUARPC_START_OFFSET, temp16 - ELUARPC_START_OFFSET, rfsc_timeout ) ) != temp16 - ELUARPC_START_OFFSET )
  {
    RFSDEBUG( "[RFS] rfsc_recv (2) error: expected %u, got %u\n", ( unsigned )( temp16 - ELUARPC_START_OFFSET ), ( unsigned )readbytes );
    return CLIENT_ERR;
  }
  return CLIENT_OK;
}

// ****************************************************************************
// Client public interface

void rfsc_setup( u8 *pbuf, p_rfsc_send rfsc_send_func, p_rfsc_recv rfsc_recv_func, timer_data_type timeout )
{
  rfsc_buffer = pbuf;
  rfsc_send = rfsc_send_func;
  rfsc_recv = rfsc_recv_func;
  rfsc_timeout = timeout;
}

void rfsc_set_timeout( timer_data_type timeout )
{
  rfsc_timeout = timeout;
}

int rfsc_open( const char* pathname, int flags, int mode )
{
  int fd;

  // Make the request
  remotefs_open_write_request( rfsc_buffer, pathname, os_open_sys_flags_to_rfs_flags( flags ), mode );

  // Send the request / get the respone
  if( rfsch_send_request_read_response() == CLIENT_ERR )
    return -1;

  // Interpret the response
  if( remotefs_open_read_response( rfsc_buffer, &fd ) == ELUARPC_ERR )
    return -1;
  return fd;
}

s32 rfsc_write( int fd, const void *buf, u32 count )
{
  // Make the request
  remotefs_write_write_request( rfsc_buffer, fd, buf, count );

  // Send the request / get the response
  if( rfsch_send_request_read_response() == CLIENT_ERR )
    return -1;
  
  // Interpret the response
  if( remotefs_write_read_response( rfsc_buffer, &count ) == ELUARPC_ERR )
    return -1;
  return ( s32 )count;
}

s32 rfsc_read( int fd, void *buf, u32 count )
{
  const u8 *resbuf;

  // Make the request
  remotefs_read_write_request( rfsc_buffer, fd, count );

  // Send the request / get the response
  if( rfsch_send_request_read_response() == CLIENT_ERR )
    return -1;

  // Interpret the response
  if( remotefs_read_read_response( rfsc_buffer, &resbuf, &count ) == ELUARPC_ERR )
    return -1;
  memcpy( buf, resbuf, count );
  return ( s32 )count;
}

s32 rfsc_lseek( int fd, s32 offset, int whence )
{
  s32 res;

  // Make the request
  remotefs_lseek_write_request( rfsc_buffer, fd, offset, os_lseek_sys_whence_to_rfs_whence( whence ) );

  // Send the request / get the response
  if( rfsch_send_request_read_response() == CLIENT_ERR )
    return -1;

  // Interpret the response
  if( remotefs_lseek_read_response( rfsc_buffer, &res ) == ELUARPC_ERR )
    return -1;
  return res;
}

int rfsc_close( int fd )
{
  int res;

  // Make the request
  remotefs_close_write_request( rfsc_buffer, fd );

  // Send the request / get the response
  if( rfsch_send_request_read_response() == CLIENT_ERR )
    return -1;

  // Interpret the response
  if( remotefs_close_read_response( rfsc_buffer, &res ) == ELUARPC_ERR )
    return -1;
  return res;
}

u32 rfsc_opendir( const char* name )
{
  u32 res;

  // Make the request
  remotefs_opendir_write_request( rfsc_buffer, name );
  if( rfsch_send_request_read_response() == CLIENT_ERR )
    return 0;

  // Interpret the response
  if( remotefs_opendir_read_response( rfsc_buffer, &res ) == ELUARPC_ERR )
    return 0;
  return res;
}

void rfsc_readdir( u32 d, const char **pname, u32 *psize, u32 *ptime )
{
  // Make the request
  remotefs_readdir_write_request( rfsc_buffer, d );
  if( rfsch_send_request_read_response() == CLIENT_ERR )
  {
    *pname = NULL;
    return;
  }

  // Interpret the response
  if( remotefs_readdir_read_response( rfsc_buffer, pname, psize, ptime ) == ELUARPC_ERR )
    *pname = NULL;
}

int rfsc_closedir( u32 d )
{
  int res;

  // Make the request
  remotefs_closedir_write_request( rfsc_buffer, d );
  if( rfsch_send_request_read_response() == CLIENT_ERR )
    return -1;

  // Interpret the response
  if( remotefs_closedir_read_response( rfsc_buffer, &res ) == ELUARPC_ERR )
    return -1;
  return res;
}  

#endif // #ifdef BUILD_RFS
