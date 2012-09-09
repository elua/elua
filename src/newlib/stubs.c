// Newlib stubs implementation

#include <reent.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include "devman.h"
#include "ioctl.h"
#include "platform.h"
#include "platform_conf.h"
#include "genstd.h"
#include "utils.h"
#include "salloc.h"

#ifdef USE_MULTIPLE_ALLOCATOR
#include "dlmalloc.h"
#else
#include <malloc.h>
#endif

// Utility function: look in the device manager table and find the index
// for the given name. Returns an index into the device structure, -1 if error.
// Also returns a pointer to the actual file name (without the device part)
static int find_dm_entry( const char* name, char **pactname )
{
  int i;
  const DM_INSTANCE_DATA* pinst;
  const char* preal;
  char tempname[ DM_MAX_DEV_NAME + 1 ];
  
  // Sanity check for name
  if( name == NULL || *name == '\0' || *name != '/' )
    return -1;
    
  // Find device name
  preal = strchr( name + 1, '/' );
  if( preal == NULL )
  {
    // This shortcut allows to register the "/" filesystem and use it like "/file.ext"
    strcpy( tempname, "/" );
    preal = name;
  }
  else
  {
    if( ( preal - name > DM_MAX_DEV_NAME ) || ( preal - name == 1 ) ) // name too short/too long
      return -1;
    memcpy( tempname, name, preal - name );
    tempname[ preal - name ] = '\0';
  }
    
  // Find device
  for( i = 0; i < dm_get_num_devices(); i ++ )
  {
    pinst = dm_get_instance_at( i );
    if( !strcasecmp( tempname, pinst->name ) )
      break;
  }
  if( i == dm_get_num_devices() )
    return -1;
    
  // Find the actual first char of the name
  preal ++;
  if( *preal == '\0' )
    return -1;
  *pactname = ( char * )preal;
  return i;  
}

// *****************************************************************************
// _open_r
int _open_r( struct _reent *r, const char *name, int flags, int mode )
{
  char* actname;
  int res, devid;
  const DM_INSTANCE_DATA *pinst;
 
  // Look for device, return error if not found or if function not implemented
  if( ( devid = find_dm_entry( name, &actname ) ) == -1 )
  {
    r->_errno = ENODEV;
    return -1; 
  }
  pinst = dm_get_instance_at( devid );
  if( pinst->pdev->p_open_r == NULL )
  {
    r->_errno = ENOSYS;
    return -1;   
  }
  
  // Device found, call its function
  if( ( res = pinst->pdev->p_open_r( r, actname, flags, mode, pinst->pdata ) ) < 0 )
    return res;
  return DM_MAKE_DESC( devid, res );
}

// *****************************************************************************
// _close_r
int _close_r( struct _reent *r, int file )
{
  const DM_INSTANCE_DATA* pinst;
  
  // Find device, check close function
  pinst = dm_get_instance_at( DM_GET_DEVID( file ) );
  if( pinst->pdev->p_close_r == NULL )
  {
    r->_errno = ENOSYS;
    return -1; 
  }
  
  // And call the close function
  return pinst->pdev->p_close_r( r, DM_GET_FD( file ), pinst->pdata );
}

// *****************************************************************************
// _fstat_r (not implemented)
int _fstat_r( struct _reent *r, int file, struct stat *st )
{
  if( ( file >= DM_STDIN_NUM ) && ( file <= DM_STDERR_NUM ) )
  {
    st->st_mode = S_IFCHR;
    return 0;
  }
  r->_errno = ENOSYS;
  return -1;
}

// *****************************************************************************
// _lseek_r
off_t _lseek_r( struct _reent *r, int file, off_t off, int whence )
{
  const DM_INSTANCE_DATA* pinst;
  
  // Find device, check close function
  pinst = dm_get_instance_at( DM_GET_DEVID( file ) );
  if( pinst->pdev->p_lseek_r == NULL )
  {
    r->_errno = ENOSYS;
    return -1; 
  }
  
  // And call the close function
  return pinst->pdev->p_lseek_r( r, DM_GET_FD( file ), off, whence, pinst->pdata );
}

// *****************************************************************************
// _read_r 
_ssize_t _read_r( struct _reent *r, int file, void *ptr, size_t len )
{
  const DM_INSTANCE_DATA* pinst;
  
  // Find device, check read function
  pinst = dm_get_instance_at( DM_GET_DEVID( file ) );
  if( pinst->pdev->p_read_r == NULL )
  {
    r->_errno = ENOSYS;
    return -1; 
  }
  
  // And call the read function
  return pinst->pdev->p_read_r( r, DM_GET_FD( file ), ptr, len, pinst->pdata );
}

// *****************************************************************************
// _write_r 
_ssize_t _write_r( struct _reent *r, int file, const void *ptr, size_t len )
{
  const DM_INSTANCE_DATA *pinst;
  
  // Find device, check write function
  pinst = dm_get_instance_at( DM_GET_DEVID( file ) );
  if( pinst->pdev->p_write_r == NULL )
  {
    r->_errno = ENOSYS;
    return -1; 
  }
  
  // And call the write function
  return pinst->pdev->p_write_r( r, DM_GET_FD( file ), ptr, len, pinst->pdata );
}

// ****************************************************************************
// _mkdir_r
int _mkdir_r( struct _reent *r, const char *path, mkdir_mode_t mode )
{
  char* actname;
  int res, devid;
  const DM_INSTANCE_DATA *pinst;

  // Look for device, return error if not found or if function not implemented
  if( ( devid = find_dm_entry( path, &actname ) ) == -1 )
  {
    r->_errno = ENODEV;
    return -1;
  }
  pinst = dm_get_instance_at( devid );
  if( pinst->pdev->p_mkdir_r == NULL )
  {
    r->_errno = EPERM;
    return -1;
  }

  // Device found, call its function
  return pinst->pdev->p_mkdir_r( r, actname - 1, mode, pinst->pdata );
}

int mkdir( const char *path, mode_t mode )
{
  return _mkdir_r( _REENT, path, mode );
}

// ****************************************************************************
// Miscellaneous functions

int _isatty_r( struct _reent* r, int fd )
{
  return 1;
}

#ifndef WIN32

int isatty( int fd )
{
  return 1;
}
#include <sys/types.h>
#include <unistd.h>

pid_t _getpid()
{
  return 0;
}

// For some very strange reason, the next function is required by the i386 platform...
pid_t getpid()
{
  return 0;
}

#include <sys/times.h>
clock_t _times_r( struct _reent* r, struct tms *buf )
{
  return 0;
}

int _unlink_r( struct _reent *r, const char *name )
{
  r->_errno = ENOSYS;
  return -1;
}

int _link_r( struct _reent *r, const char *c1, const char *c2 )
{
  r->_errno = ENOSYS;
  return -1;
}

#include <sys/time.h>
int _gettimeofday_r( struct _reent *r, struct timeval *tv, void *tz )
{
  r->_errno = ENOSYS;
  return -1;  
}

#include <stdlib.h>
void _exit( int status )
{
  while( 1 );
}

int _kill( int pid, int sig )
{
  return -1;
}
#endif

// If LUA_NUMBER_INTEGRAL is defined, "redirect" printf/scanf calls to their 
// integer counterparts
#ifdef LUA_NUMBER_INTEGRAL
int _vfprintf_r( struct _reent *r, FILE *stream, const char *format, va_list ap )
{
  return _vfiprintf_r( r, stream, format, ap );
}

extern int _svfiprintf_r( struct _reent *r, FILE *stream, const char *format, va_list ap );
int _svfprintf_r( struct _reent *r, FILE *stream, const char *format, va_list ap )
{
  return _svfiprintf_r( r, stream, format, ap );
}

extern int __svfiscanf_r(struct _reent *,FILE *, _CONST char *,va_list);
int __svfscanf_r( struct _reent *r, FILE *stream, const char *format, va_list ap )
{
  return __svfiscanf_r( r, stream, format, ap );
}

extern int __ssvfiscanf_r(struct _reent *,FILE *, _CONST char *,va_list);
int __ssvfscanf_r( struct _reent *r, FILE *stream, const char *format, va_list ap )
{
  return __ssvfiscanf_r( r, stream, format, ap );
}

#endif // #ifdef LUA_NUMBER_INTEGRAL

// ****************************************************************************
// Allocator support

// _sbrk_r (newlib) / elua_sbrk (multiple)
static char *heap_ptr; 
static int mem_index;

#ifdef USE_MULTIPLE_ALLOCATOR
void* elua_sbrk( ptrdiff_t incr )
#else
void* _sbrk_r( struct _reent* r, ptrdiff_t incr )
#endif
{
  void* ptr;
      
  // If increment is negative, return -1
  if( incr < 0 )
    return ( void* )-1;
    
  // Otherwise ask the platform about our memory space (if needed)
  // We do this for all our memory spaces
  while( 1 )
  {
    if( heap_ptr == NULL )  
    {
      if( ( heap_ptr = ( char* )platform_get_first_free_ram( mem_index ) ) == NULL )
      {
        ptr = ( void* )-1;
        break;
      }
    }
      
    // Do we have space in the current memory space?
    if( heap_ptr + incr > ( char* )platform_get_last_free_ram( mem_index )  ) 
    {
      // We don't, so try the next memory space
      heap_ptr = NULL;
      mem_index ++;
    }
    else
    {
      // Memory found in the current space
      ptr = heap_ptr;
      heap_ptr += incr;
      break;
    }
  }  

  return ptr;
} 

// mallinfo()
struct mallinfo mallinfo()
{
#ifdef USE_MULTIPLE_ALLOCATOR
  return dlmallinfo();
#else
  return _mallinfo_r( _REENT );
#endif
} 

#if defined( USE_MULTIPLE_ALLOCATOR ) || defined( USE_SIMPLE_ALLOCATOR )
// Redirect all allocator calls to our dlmalloc/salloc 

#ifdef USE_MULTIPLE_ALLOCATOR
#define CNAME( func ) dl##func
#else
#define CNAME( func ) s##func
#endif

void* _malloc_r( struct _reent* r, size_t size )
{
  return CNAME( malloc )( size );
}

void* _calloc_r( struct _reent* r, size_t nelem, size_t elem_size )
{
  return CNAME( calloc )( nelem, elem_size );
}

void _free_r( struct _reent* r, void* ptr )
{
  CNAME( free )( ptr );
}

void* _realloc_r( struct _reent* r, void* ptr, size_t size )
{
  return CNAME( realloc )( ptr, size );
}

#endif // #ifdef USE_MULTIPLE_ALLOCATOR

// *****************************************************************************
// eLua stubs (not Newlib specific)

#if !defined( BUILD_CON_GENERIC ) && !defined( BUILD_CON_TCP )

// Set send/recv functions
void std_set_send_func( p_std_send_char pfunc )
{
}

void std_set_get_func( p_std_get_char pfunc )
{
}

const DM_DEVICE* std_get_desc()
{
  return NULL;
}

#endif // #if !defined( BUILD_CON_GENERIC ) && !defined( BUILD_CON_TCP )

// ****************************************************************************
// memcpy is broken on AVR32's Newlib, so implement a simple version here
// same goes for strcmp apparently
#ifdef FORAVR32
void* memcpy( void *dst, const void* src, size_t len )
{
  char *pdest = ( char* )dst;
  const char* psrc = ( const char* )src;
  
  while( len )
  {
    *pdest ++ = *psrc ++;
    len --;
  }
  return dst;
}

int strcmp(const char *s1, const char *s2)
{
  while( *s1 == *s2++ ) 
  {
    if( *s1++ == '\0' )
      return 0;
  }
  if( *s1 == '\0' ) 
    return -1;
  if( *--s2 == '\0' ) 
    return 1;
  return ( unsigned char )*s1 - ( unsigned char )*s2;
}

#endif // #ifdef FORAVR32
