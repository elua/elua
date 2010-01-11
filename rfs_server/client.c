// Remote filesystem client

#include <string.h>
#include "remotefs.h"
#include "client.h"
#include "os_io.h"

// ****************************************************************************
// Client local data

static u8 *rfsc_buffer;
static p_rfsc_send rfsc_send;
static p_rfsc_recv rfsc_recv;
static u32 rfsc_timeout;

// ****************************************************************************
// Client helpers

static int rfsch_send_request_read_response()
{
  u16 temp16;

  // Send request
  if( remotefs_get_packet_size( rfsc_buffer, &temp16 ) == REMOTEFS_ERR )
    return CLIENT_ERR;
  if( rfsc_send( rfsc_buffer, temp16 ) != temp16 )
    return CLIENT_ERR;
  
  // Get response
  // First the length, then the rest of the data
  if( rfsc_recv( rfsc_buffer, RFS_START_OFFSET, rfsc_timeout ) != RFS_START_OFFSET )
    return CLIENT_ERR;
  if( remotefs_get_packet_size( rfsc_buffer, &temp16 ) == REMOTEFS_ERR )
    return CLIENT_ERR;
  if( rfsc_recv( rfsc_buffer + RFS_START_OFFSET, temp16 - RFS_START_OFFSET, rfsc_timeout ) != temp16 - RFS_START_OFFSET )
    return CLIENT_ERR;
  return CLIENT_OK;
}

// ****************************************************************************
// Client public interface

void rfsc_setup( u8 *pbuf, p_rfsc_send rfsc_send_func, p_rfsc_recv rfsc_recv_func, u32 timeout )
{
  rfsc_buffer = pbuf;
  rfsc_send = rfsc_send_func;
  rfsc_recv = rfsc_recv_func;
  rfsc_timeout = timeout;
}

void rfsc_set_timeout( u32 timeout )
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
  if( remotefs_open_read_response( rfsc_buffer, &fd ) == REMOTEFS_ERR )
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
  if( remotefs_write_read_response( rfsc_buffer, &count ) == REMOTEFS_ERR )
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
  if( remotefs_read_read_response( rfsc_buffer, &resbuf, &count ) == REMOTEFS_ERR )
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
  if( remotefs_lseek_read_response( rfsc_buffer, &res ) == REMOTEFS_ERR )
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
  if( remotefs_close_read_response( rfsc_buffer, &res ) == REMOTEFS_ERR )
    return -1;
  return res;
}

