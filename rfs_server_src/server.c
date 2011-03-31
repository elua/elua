// Remote filesystem server implementation

#include <string.h>
#include <stdlib.h>
#include "server.h"
#include "remotefs.h"
#include "eluarpc.h"
#include "type.h"
#include "os_io.h"
#include "log.h"

static char* server_basedir;
static char server_fullname[ PLATFORM_MAX_FNAME_LEN + 1 ];

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
  if( remotefs_open_read_request( p, &filename, &flags, &mode ) == ELUARPC_ERR )
  {
    log_msg( "server_open: unable to read request\n" );
    return SERVER_ERR;
  }
  // Get real filename
  server_fullname[ 0 ] = server_fullname[ PLATFORM_MAX_FNAME_LEN ] = 0;
  strncpy( server_fullname, server_basedir, PLATFORM_MAX_FNAME_LEN );
  if( filename && strlen( filename ) > 0 )
  {
    if( server_fullname[ strlen( server_fullname ) - 1 ] != PLATFORM_PATH_SEPARATOR )
      strncat( server_fullname, separator, PLATFORM_MAX_FNAME_LEN );
    strncat( server_fullname, filename, PLATFORM_MAX_FNAME_LEN );
  }
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
  if( remotefs_write_read_request( p, &fd, &buf, &count ) == ELUARPC_ERR )
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
  if( remotefs_read_read_request( p, &fd, &count ) == ELUARPC_ERR )
  {
    log_msg( "server_read: unable to read request\n" );
    return SERVER_ERR;
  }
  log_msg( "server_read: fd = %d, count = %u\n", fd, ( unsigned )count );
  count = ( u32 )os_read( fd, p + ELUARPC_READ_BUF_OFFSET, count );
  log_msg( "server_read: OS response is %u\n", ( unsigned )count );
  remotefs_read_write_response( p, count );
  return SERVER_OK;
}

static int server_close( u8 *p )
{
  int fd;
  
  log_msg( "server_close: request handler starting\n" );
  if( remotefs_close_read_request( p, &fd ) == ELUARPC_ERR )
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

  log_msg( "server_lseek: request handler starting\n" );
  if( remotefs_lseek_read_request( p, &fd, &offset, &whence ) == ELUARPC_ERR )
  {
    log_msg( "server_lseek: unable to read request\n" );
    return SERVER_ERR;
  }
  log_msg( "server_lseek: fd = %d, offset = %d, whence = %d\n", fd, ( int )offset, whence );
  offset = os_lseek( fd, offset, whence );
  log_msg( "server_lseek: OS response is %d\n", ( int )offset );
  remotefs_lseek_write_response( p, offset );
  return SERVER_OK;
}

static int server_opendir( u8 *p )
{
  const char* name;
  u32 d;
  char separator[ 2 ] = { PLATFORM_PATH_SEPARATOR, 0 };

  log_msg( "server_opendir: request handler starting\n" );
  if( remotefs_opendir_read_request( p, &name ) == ELUARPC_ERR )
  {
    log_msg( "server_opendir: unable to read request\n" );
    return SERVER_ERR;
  }
  // Get real filename
  server_fullname[ 0 ] = server_fullname[ PLATFORM_MAX_FNAME_LEN ] = 0;
  strncpy( server_fullname, server_basedir, PLATFORM_MAX_FNAME_LEN );
  if( name && strlen( name ) > 0 )
  {
    if( server_fullname[ strlen( server_fullname ) - 1 ] != PLATFORM_PATH_SEPARATOR )
      strncat( server_fullname, separator, PLATFORM_MAX_FNAME_LEN );
    strncat( server_fullname, name, PLATFORM_MAX_FNAME_LEN );
  }
  log_msg( "server_opendir: full dirname is %s\n", server_fullname );
  d = os_opendir( server_fullname );
  log_msg( "server_opendir: OS response is %08X\n", d );
  remotefs_opendir_write_response( p, d );
  return SERVER_OK;
}

static int server_readdir( u8 *p )
{
  const char* name;
  u32 fsize = 0, d;
  int fd;
  char separator[ 2 ] = { PLATFORM_PATH_SEPARATOR, 0 };

  log_msg( "server_readdir: request handler starting\n" );
  if( remotefs_readdir_read_request( p, &d ) == ELUARPC_ERR )
  {
    log_msg( "server_readdir: unable to read request\n" );
    return SERVER_ERR;
  }
  log_msg( "server_readdir: DIR = %08X\n", d );
  os_readdir( d, &name );
  if( name )
  {
    // Need to compute size now
    // Get real filename
    server_fullname[ 0 ] = server_fullname[ PLATFORM_MAX_FNAME_LEN ] = 0;
    strncpy( server_fullname, server_basedir, PLATFORM_MAX_FNAME_LEN );
    if( name && strlen( name ) > 0 )
    {
      if( server_fullname[ strlen( server_fullname ) - 1 ] != PLATFORM_PATH_SEPARATOR )
        strncat( server_fullname, separator, PLATFORM_MAX_FNAME_LEN );
      strncat( server_fullname, name, PLATFORM_MAX_FNAME_LEN );
    }
    fd = os_open( server_fullname, RFS_OPEN_FLAG_RDONLY, 0 );
    if( fd )
    {
      fsize = os_lseek( fd, 0, RFS_LSEEK_END );
      os_close( fd );
    }
    else
    {
      log_msg( "server_readdir: unable to open file %s\n", server_fullname );
      name = NULL;
    }
  }
  log_msg( "server_readdir: OS response is fname = %s, fsize = %u\n", name, ( unsigned )fsize );
  remotefs_readdir_write_response( p, name, fsize, 0 );
  return SERVER_OK;
}

static int server_closedir( u8 *p )
{
  u32 d;
  int res;

  log_msg( "server_closedir: request handler starting\n" );
  if( remotefs_closedir_read_request( p, &d ) == ELUARPC_ERR )
  {
    log_msg( "server_closedir: unable to read request\n" );
    return SERVER_ERR;
  }
  log_msg( "server_closedir: DIR = %08X\n", d );
  res = os_closedir( d );
  log_msg( "server_closedir: OS response is %d\n", res );
  remotefs_closedir_write_response( p, d );
  return SERVER_OK;
}

// *****************************************************************************
// Server public interface

static const p_server_handler server_handlers[] = 
{ 
  server_open, server_write, server_read, server_close, server_lseek, server_opendir, server_readdir, server_closedir
};

void server_setup( const char* basedir )
{
  server_basedir = strdup( basedir );
}

void server_cleanup()
{
  free( server_basedir );
  server_basedir = NULL;
}
int server_execute_request( u8 *pdata )
{
  u8 req;
  
  // Decode request
  if( eluarpc_get_request_id( pdata, &req ) == ELUARPC_ERR )
    return SERVER_ERR;
  log_msg( "server_execute_request: got request with ID %d\n", req );
  if( req >= RFS_OP_FIRST && req <= RFS_OP_LAST ) 
    return server_handlers[ req - RFS_OP_FIRST ]( pdata );
  else
    return SERVER_ERR;

}

