// Console (stdin/stdout/stderr) over TCP

#include "platform_conf.h"
#ifdef BUILD_CON_TCP

#include "type.h"
#include "devman.h"
#include "genstd.h"
#include "elua_net.h"
#include "utils.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>

// 'read'
static _ssize_t std_read( struct _reent *r, int fd, void* vptr, size_t len, void *pdata )
{
  int sock;
  elua_net_size pktsize;
  size_t actsize, j;
  char* lptr = ( char* )vptr;

  // Check file number
  if( fd != DM_STDIN_NUM )
  {
    r->_errno = EINVAL;
    return -1;
  }      

  // Get (and wait for) socket
  while( ( sock = elua_net_get_telnet_socket() ) == - 1 );
  
  // Read data
  actsize = 0;
  while( 1 )
  {
    pktsize = elua_net_recv( sock, lptr, len, -1, 0, 0 );
    // Check EOF
    for( j = 0; j < pktsize; j ++ )
      if( lptr[ j ] == STD_CTRLZ_CODE )
        return 0;
    actsize += pktsize;
    if( actsize >= len )
    {
      actsize = len;
      break;
    }
    // Is this the final packet?
    if( pktsize >= 2 && lptr[ pktsize - 2 ] == '\r' && lptr[ pktsize - 1 ] == '\n' ) // final packet
      break;    
    lptr += pktsize;
    len -= pktsize;
  }
  return actsize;
}

// 'write'
static _ssize_t std_write( struct _reent *r, int fd, const void* vptr, size_t len, void *pdata )
{   
  int sock;
  
  // Check file number
  if( ( fd != DM_STDOUT_NUM ) && ( fd != DM_STDERR_NUM ) )
  {
    r->_errno = EINVAL;
    return -1;
  }  
  
  // Get (and wait for) socket
  while( ( sock = elua_net_get_telnet_socket() ) == - 1 );  
  
  // Send data
  elua_net_send( sock, vptr, len );
  return len;
}

// Set send/recv functions
void std_set_send_func( p_std_send_char pfunc )
{
}

void std_set_get_func( p_std_get_char pfunc )
{
}

static const DM_DEVICE std_device = 
{
  NULL,                 // open
  NULL,                 // close
  std_write,            // write
  std_read,             // read
  NULL,                 // lseek
  NULL,                 // opendir
  NULL,                 // readdir
  NULL,                 // closedir
  NULL,                 // getaddr
  NULL                  // mkdir
};


int std_register()
{
  return dm_register( STD_DEV_NAME, NULL, &std_device );
}

#endif // #ifdef BUILD_CON_TCP
