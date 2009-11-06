#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <errno.h>
#include <string.h>
#include <alloca.h>

/* FIXME:  I know not all of the above is necessary, should pare it down sometime */

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include "luarpc_rpc.h"

void transport_open( Transport *tpt, const char *path );


#ifdef LUARPC_ENABLE_SERIAL

/* Setup Transport */
void transport_init (Transport *tpt)
{
  tpt->fd = INVALID_TRANSPORT;
}

void transport_open( Transport *tpt, const char *path )
{
  struct termios options;
  struct exception e;
  
  tpt->fd = open(path , O_RDWR | O_NOCTTY | O_NDELAY );
  
  if( tpt->fd == INVALID_TRANSPORT)
  {
    e.errnum = errno;
    e.type = fatal;
    Throw( e );
  }
    
  tcgetattr( tpt->fd, &options);

  cfsetispeed( &options, B115200 );
  cfsetospeed( &options, B115200 );

  options.c_cflag     |= (CLOCAL | CREAD);
  
  /* raw processing */
  options.c_lflag     &= ~(ICANON | ECHO | ECHOE | ISIG);
  options.c_oflag     &= ~OPOST;
  
  /* 8N1 */
  options.c_cflag     &= ~PARENB;
  options.c_cflag     &= ~CSTOPB;
  options.c_cflag     &= ~CSIZE;
  options.c_cflag     |= CS8;
  
  options.c_cc[VMIN]  = 1;
  options.c_cc[VTIME] = 10;

  tcsetattr(tpt->fd, TCSANOW, &options);
  fcntl(tpt->fd, F_SETFL, 0);
}

/* Open Listener / Server */
void transport_open_listener(lua_State *L, ServerHandle *handle)
{
  check_num_args (L,2); /* 1st arg is path, 2nd is handle */
  if (!lua_isstring (L,1))
    my_lua_error (L,"first argument must be serial serial port");

  transport_open( &handle->ltpt, lua_tostring (L,1) );
    
  while( transport_readable ( &handle->ltpt ) == 0 ); /* wait for incoming data */
}

/* Open Connection / Client */
int transport_open_connection(lua_State *L, Handle *handle)
{ 
  check_num_args (L,2); /* 1st arg is path, 2nd is handle */
  if (!lua_isstring (L,1))
    my_lua_error (L,"first argument must be serial serial port");

  transport_open( &handle->tpt, lua_tostring (L,1) );
  
  return 1;
}

/* Accept Connection */
void transport_accept (Transport *tpt, Transport *atpt)
{
  struct exception e;
  TRANSPORT_VERIFY_OPEN;
  atpt->fd = tpt->fd;
}


/* Read & Write to Transport */
void transport_read_buffer (Transport *tpt, u8 *buffer, int length)
{
  int n;
  struct exception e;
  TRANSPORT_VERIFY_OPEN;
  
  while( length > 0 )
  {
    TRANSPORT_VERIFY_OPEN;

    n = read( tpt->fd, ( void * )buffer, length );
    
    /* error handling */
    if( n == 0 )
    {
      e.errnum = ERR_NODATA;
      e.type = nonfatal;
      Throw( e );
    }
    
    if( n < 0 )
    {
      e.errnum = errno;
      e.type = fatal;
      Throw( e );
    }
   
    buffer += n;
    length -= n;
  }
}

void transport_write_buffer( Transport *tpt, const u8 *buffer, int length )
{
  int n;
  struct exception e;
  TRANSPORT_VERIFY_OPEN;

  n = write( tpt->fd, buffer,length );
  
  if ( n != length )
  {
    e.errnum = errno;
    e.type = fatal;
    Throw( e );
  }
}

/* Check if data is available on connection without reading:
    - 1 = data available, 0 = no data available */
int transport_readable (Transport *tpt)
{
  struct exception e;
  fd_set rdfs;
  int ret;
  struct timeval tv;

  if (tpt->fd == INVALID_TRANSPORT)
    return 0;
  
  FD_ZERO (&rdfs);
  FD_SET (tpt->fd, &rdfs);
  
  /* Wait up to five seconds. */
  tv.tv_sec = 5;
  tv.tv_usec = 0;

  ret = select( tpt->fd+1, &rdfs, NULL, NULL, &tv );
  
  if ( ret < 0 )
  {
    e.errnum = errno;
    e.type = fatal;
    Throw( e );
  }
    
  return ( ret > 0 );
}

/* Check if transport is open:
    1 = connection open, 0 = connection closed */
int transport_is_open (Transport *tpt)
{
  return (tpt->fd != INVALID_TRANSPORT);
}

/* Shut down connection */
void transport_close (Transport *tpt)
{
  if (tpt->fd != INVALID_TRANSPORT)
  {
    /* close (tpt->fd); -- not closing for now since atpt and ltpt use same fd,
                           should use some method to detect whether one has 
                           already dropped, and properly close out on exit */
    
    /* Send break to the other side to indicate to opposing side that connection is ending */
    tcsendbreak(tpt->fd, 0);
    tpt->fd = INVALID_TRANSPORT;
  }
}

#endif /* LUARPC_ENABLE_SERIAL */
