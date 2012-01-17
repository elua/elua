// Generic stdin/stdout/stderr support functions

#include "platform_conf.h"
#ifdef BUILD_CON_GENERIC

#include "type.h"
#include "devman.h"
#include "genstd.h"
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include "utils.h"

static p_std_send_char std_send_char_func;
static p_std_get_char std_get_char_func;

// 'read'
static _ssize_t std_read( struct _reent *r, int fd, void* vptr, size_t len )
{
  int i, c;
  char* ptr = ( char* )vptr;
  
  // Check pointers
  if( !std_send_char_func || !std_get_char_func )
  {
    r->_errno = EBADF;
    return -1;  
  }
  
  // Check file number
  if( ( fd < DM_STDIN_NUM ) || ( fd > DM_STDERR_NUM ) )
  {
    r->_errno = EBADF;
    return -1;
  }  
  if( fd != DM_STDIN_NUM )
  {
    r->_errno = EINVAL;
    return -1;
  }      
  
  i = 0;
  do {
    c = std_get_char_func( STD_INFINITE_TIMEOUT );
    switch( c )
    {
    case STD_CTRLZ_CODE:  // End of file
      return 0;

    case -1:              // UART overrun/failure - return chars so far
      return i;

    case 8: case 0x7F:    // Backspace
      if( i > 0 )
      {
        i --;        
        std_send_char_func( DM_STDOUT_NUM, 8 );      
        std_send_char_func( DM_STDOUT_NUM, ' ' );      
        std_send_char_func( DM_STDOUT_NUM, 8 );                    
      }      
      break;

    case '\r': case '\n':
      // Allow either CR or LF as line terminator: in either case
      // output CRLF and return the line of input with a \n on the end.
      std_send_char_func( DM_STDOUT_NUM, '\r');
      std_send_char_func( DM_STDOUT_NUM, '\n');
      ptr[ i++ ] = '\n';
      return i;

    default:
      // Ignore control characters
      if( !isprint( c ) )
        continue;
      // Echo the character
      std_send_char_func( DM_STDOUT_NUM, c );
      ptr[ i ++ ] = c;
      break;
    }
  } while( i < len );
  return i;
}

// 'write'
static _ssize_t std_write( struct _reent *r, int fd, const void* vptr, size_t len )
{   
  int i;
  const char* ptr = ( const char* )vptr;
  
  // Check pointers
  if( !std_send_char_func || !std_get_char_func )
  {
    r->_errno = EBADF;
    return -1;  
  }
    
  // Check file number
  if( ( fd < DM_STDIN_NUM ) || ( fd > DM_STDERR_NUM ) )
  {
    r->_errno = EBADF;
    return -1;
  }    
  if( ( fd != DM_STDOUT_NUM ) && ( fd != DM_STDERR_NUM ) )
  {
    r->_errno = EINVAL;
    return -1;
  }  
  
  for( i = 0; i < len; i ++ ) 
  {
    if( ptr[ i ] == '\n' )
      std_send_char_func( fd, '\r' );
    std_send_char_func( fd, ptr[ i ] ); 
  }
  return len;
}

// Set send/recv functions
void std_set_send_func( p_std_send_char pfunc )
{
  std_send_char_func = pfunc;
}

void std_set_get_func( p_std_get_char pfunc )
{
  std_get_char_func = pfunc;
}

// Our UART device descriptor structure
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

#endif // #ifdef BUILD_CON_GENERIC
