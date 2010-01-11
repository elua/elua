// Remote filesystem server implementation

#include <string.h>
#include "server.h"
#include "remotefs.h"
#include "type.h"
#include "os_io.h"
#include "log.h"

static char* server_basedir;
static char server_fullname[ PLATFORM_MAX_FNAME_LEN + 1 ];

#ifdef DEBUG
#include <stdio.h>
#define LOG0(str) printf( str"\n" )
#define LOG1(str, arg1) printf( str"\n", arg1 )
#define LOG2(str, arg1, arg2) printf( str"\n", arg1, arg2 )
#define LOG3(str, arg1, arg2, arg3) printf( str"\n", arg1, arg2, arg3 )
#else
#define LOG0(str)
#define LOG1(str, arg1)
#define LOG2(str, arg1, arg2)
#define LOG3(str, arg1, arg2, arg3)
#endif

typedef int ( *p_server_handler )( u8 *p );

// *****************************************************************************
// Internal helpers: execute the given request, build the response

static int server_open( u8 *p )
{
  const char *filename;
  int mode, flags, fd;
  char separator[ 2 ] = { PLATFORM_PATH_SEPARATOR, 0 };
  
  // Validate request
  log_msg( "server_open: request handler starting\n" );
  if( remotefs_open_read_request( p, &filename, &flags, &mode ) == REMOTEFS_ERR )
  {
    log_msg( "server_open: unable to read request\n" );
    return SERVER_ERR;
  }
  // Change all the flags to their regular counterpart
  server_fullname[ 0 ] = server_fullname[ PLATFORM_MAX_FNAME_LEN ] = 0;
  strncpy( server_fullname, server_basedir, PLATFORM_MAX_FNAME_LEN );
  if( server_fullname[ strlen( server_fullname ) - 1 ] != PLATFORM_PATH_SEPARATOR )
    strncat( server_fullname, separator, PLATFORM_MAX_FNAME_LEN );
  strncat( server_fullname, filename, PLATFORM_MAX_FNAME_LEN );
  log_msg( "server_open: full file path is %s\n", server_fullname ); 
  fd = os_open( server_fullname, flags, mode );
  log_msg( "server_open: OS file handler is %d\n", fd );
  remotefs_open_write_response( p, fd );
  return SERVER_OK;
}

static int server_write( u8 *p )
{
  int fd;
  const void *buf;
  u32 count;
  
  log_msg( "server_write: request handler starting\n" );
  if( remotefs_write_read_request( p, &fd, &buf, &count ) == REMOTEFS_ERR )
  {
    log_msg( "server_write: unable to read request\n" );
    return SERVER_ERR;
  }
  log_msg( "server_write: fd = %d, buf = %p, count = %u\n", fd, buf, ( unsigned )count );
  count = ( u32 )os_write( fd, buf, count );
  log_msg( "server_write: OS response is %u\n", ( unsigned )count );
  remotefs_write_write_response( p, count );
  return SERVER_OK;
}

static int server_read( u8 *p )
{
  int fd;
  u32 count;
  
  log_msg( "server_read: request handler starting\n" );
  if( remotefs_read_read_request( p, &fd, &count ) == REMOTEFS_ERR )
  {
    log_msg( "server_read: unable to read request\n" );
    return SERVER_ERR;
  }
  log_msg( "server_read: fd = %d, count = %u\n", fd, ( unsigned )count );
  count = ( u32 )os_read( fd, p + RFS_READ_BUF_OFFSET, count );
  log_msg( "server_read: OS response is %u\n", ( unsigned )count );
  remotefs_read_write_response( p, count );
  return SERVER_OK;
}

static int server_close( u8 *p )
{
  int fd;
  
  log_msg( "server_close: request handler starting\n" );
  if( remotefs_close_read_request( p, &fd ) == REMOTEFS_ERR )
  {
    log_msg( "server_close: unable to read request\n" );
    return SERVER_ERR;
  }
  log_msg( "server_close: fd = %d\n", fd );
  fd = os_close( fd );
  log_msg( "server_close: OS response is %d\n", fd );
  remotefs_close_write_response( p, fd );
  return SERVER_OK;
}

static int server_lseek( u8 *p )
{
  int fd, whence;
  s32 offset;

  log_msg( "server_lseek: request handler starting" );
  if( remotefs_lseek_read_request( p, &fd, &offset, &whence ) == REMOTEFS_ERR )
  {
    log_msg( "server_lseek: unable to read request" );
    return SERVER_ERR;
  }
  log_msg( "server_lseek: fd = %d, offset = %d, whence = %d\n", fd, ( int )offset, whence );
  offset = os_lseek( fd, offset, whence );
  log_msg( "server_lseek: OS response is %d\n", ( int )offset );
  remotefs_lseek_write_response( p, offset );
  return SERVER_OK;
}

// *****************************************************************************
// Server public interface

static const p_server_handler server_handlers[] = 
{ 
  server_open, server_write, server_read, server_close, server_lseek
};

void server_setup( const char* basedir )
{
  server_basedir = strdup( basedir );
}

int server_execute_request( u8 *pdata )
{
  u8 req;
  
  // Decode request
  if( remotefs_get_request_id( pdata, &req ) == REMOTEFS_ERR )
    return SERVER_ERR;
  log_msg( "server_execute_request: got request with ID %d\n", req );
  if( req >= RFS_OP_FIRST && req <= RFS_OP_LAST ) 
    return server_handlers[ req - RFS_OP_FIRST ]( pdata );
  else
    return SERVER_ERR;

}

