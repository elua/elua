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

// ****************************************************************************
// Local functions and helpers

// TELNET specific data
#define TELNET_IAC_CHAR         255
#define TELNET_IAC_3B_FIRST     251
#define TELNET_IAC_3B_LAST      254
#define TELNET_SB_CHAR          250
#define TELNET_SE_CHAR          240
#define TELNET_EOF              236

// Utility function for TELNET: parse input buffer, skipping over
// TELNET specific sequences
// Returns the length of the buffer after processing
static unsigned stdh_telnet_handle_input( char *buf, unsigned buflen )
{
  int skip;
  char *pdata = buf;
  unsigned datalen = buflen;

  while( ( pdata < buf + buflen ) && datalen )
  {
    if( *pdata != TELNET_IAC_CHAR ) // regular char, skip it
      pdata ++;
    else
    {
      skip = 1;
      if( pdata[ 1 ] == TELNET_IAC_CHAR ) // this is actually a TELNET_IAC_CHAR (data)
        pdata ++;
      else if( pdata[ 1 ] >= TELNET_IAC_3B_FIRST && pdata[ 1 ] <= TELNET_IAC_3B_LAST )
        skip = 3; // option negotiation, remove all 3 chars
      else if( pdata[ 1 ] == TELNET_SB_CHAR ) // suboption negotiation, ignore until SE is found
      {
        while( pdata[ skip ] != TELNET_SE_CHAR && pdata[ skip ] != 0 )
          skip ++;
        if( pdata[ skip ] == TELNET_SE_CHAR )
          skip ++;
      }
      else if( pdata[ 1 ] == TELNET_EOF ) // replace with EOF, remove one char from input
        *pdata ++ = STD_CTRLZ_CODE;
      datalen -= skip;
      memmove( pdata, pdata + skip, datalen - ( pdata - buf ) );
    }
  }

  return datalen;
}

// ****************************************************************************
// Device interface

// 'read'
static _ssize_t std_read( struct _reent *r, int fd, void* vptr, size_t len )
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
    pktsize = elua_net_recv( sock, lptr, len, PLATFORM_TIMER_SYS_ID, PLATFORM_TIMER_INF_TIMEOUT );
    pktsize = stdh_telnet_handle_input( lptr, pktsize );
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
static _ssize_t std_write( struct _reent *r, int fd, const void* vptr, size_t len )
{   
  int sock;
  unsigned crt, lastn;
  const char *pdata = ( const char* )vptr;
  
  // Check file number
  if( ( fd != DM_STDOUT_NUM ) && ( fd != DM_STDERR_NUM ) )
  {
    r->_errno = EINVAL;
    return -1;
  }  
  
  // If socket not active, just ignore request
  while( ( sock = elua_net_get_telnet_socket() ) == -1 );
  
  // Send data, transforming '\n' to '\r\n' along the way
  crt = lastn = 0;
  while( crt < len )
  {
    if( pdata[ crt ] == '\n' && ( crt == 0 || pdata[ crt - 1 ] != '\r' ) )
    {
      // Send the data until '\n' if needed
      if( crt - lastn > 0 )
        elua_net_send( sock, pdata + lastn, crt - lastn );
      // Send '\r\n' instead of '\n'
      elua_net_send( sock, "\r\n", 2 );
      crt ++;
      lastn = crt;
    }
    else
      crt ++;
  }
  if( crt - lastn > 0 ) // flush buffer if needed
    elua_net_send( sock, pdata + lastn, crt - lastn );
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
  STD_DEV_NAME,
  NULL,                 // open
  NULL,                 // close
  std_write,            // write
  std_read,             // read
  NULL,                 // lseek
  NULL,                 // opendir
  NULL,                 // readdir
  NULL                  // closedir
};


const DM_DEVICE* std_get_desc()
{
  return &std_device;
}

#endif // #ifdef BUILD_CON_TCP
