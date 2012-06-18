// Device manager interface for Newlib (stubs and additional IOCTL implementation)

#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <reent.h>
#include <errno.h>
#include <stdlib.h>
#include "devman.h"
#include "genstd.h"
#include "common.h"
#include "platform_conf.h"

static DM_INSTANCE_DATA dm_list[ DM_MAX_DEVICES ];            // list of devices
static int dm_num_devs;                                       // number of devices

// "Shared" variables: these can be used by any FS that implements 'ls' via opendir/readdir/closedir
struct dm_dirent dm_shared_dirent;
char dm_shared_fname[ DM_MAX_FNAME_LENGTH + 1 ];

// Register a device
// Returns the index of the device in the device table
int dm_register( const char *name, void *pdata, const DM_DEVICE *pdev )
{
  int i;
  
  if( pdev == NULL )
    return DM_ERR_INVALID_OPS;

  // First char of the name must be '/'
  if( name == NULL || *name != '/' || strlen( name ) > DM_MAX_DEV_NAME )
    return DM_ERR_INVALID_NAME;
  
  // Check if the device is not already registered
  for( i = 0; i < dm_num_devs; i ++ )
    if( !strcasecmp( name, dm_list[ i ].name ) )
      return DM_ERR_ALREADY_REGISTERED;
  
  // Check for space
  if( dm_num_devs == DM_MAX_DEVICES )
    return DM_ERR_NO_SPACE;
    
  // Register it now
  dm_list[ i ].name = name;
  dm_list[ i ].pdata = pdata;
  dm_list[ i ].pdev = pdev;
  dm_num_devs ++;
  return i;
}

// Helper: get a device ID from its name
// Also return a pointer to the remaining part of the name as side effect
static int dm_device_id_from_name( const char* name, const char **rest )
{
  unsigned i, pos;
  unsigned matchlen = 0;

  if( rest )
    *rest = NULL;
  for( i = pos = 0; i < dm_num_devs; i ++ )
    if( !strncasecmp( name, dm_list[ i ].name, strlen( dm_list[ i ].name ) ) && strlen( dm_list[ i ].name ) > matchlen )
    {
      matchlen = strlen( dm_list[ i ].name );
      pos = i;
    }
  if( matchlen == 0 )
    return DM_ERR_NO_DEVICE;
  if( rest )
    *rest = name + strlen( dm_list[ pos ].name );
  return pos;
}

// Unregister a device
// Returns 0 for OK or an error code if error
int dm_unregister( const char* name )
{
  int i;
  
  if( name == NULL || *name == '\0' || *name != '/' || strlen( name ) > DM_MAX_DEV_NAME )
    return DM_ERR_INVALID_NAME;
      
  // Check if the device is already registered
  for( i = 0; i < dm_num_devs; i ++ )
    if( !strcasecmp( name, dm_list[ i ].name ) )
      break;
  if( i == dm_num_devs )
    return DM_ERR_NOT_REGISTERED;
  
  // Remove it
  if( i != dm_num_devs - 1 )
    memmove( dm_list + i, dm_list + i + 1, sizeof( DM_INSTANCE_DATA ) * ( dm_num_devs - i - 1 ) );
  dm_num_devs --;
  return DM_OK;
}

// Get a device entry
const DM_DEVICE* dm_get_device_at( int idx )
{
  if( idx < 0 || idx >= dm_num_devs )
    return NULL;
  return dm_list[ idx ].pdev;
}

// Get an instance
const DM_INSTANCE_DATA* dm_get_instance_at( int idx )
{
  if( idx < 0 || idx >= dm_num_devs )
    return NULL;
  return dm_list + idx;
}

// Returns the number of registered devices
int dm_get_num_devices()
{
  return dm_num_devs;
}

// Initialize device manager
// This initializes the standard descriptors (stdin, stdout, stderr)
// At this point it is assumed that the std device (usually UART) is already initialized
int dm_init() 
{
  std_register();
#ifndef BUILD_CON_TCP         // we need buffering on stdout for console over TCP
  setbuf( stdout, NULL );
#endif
  return DM_OK;
}

// Open a directory and return its descriptor
DM_DIR* dm_opendir( const char* dirname )
{
  const char* rest;
  const DM_DEVICE *pdev;
  DM_DIR *d;
  int pos;
  void *data;

  if( ( pos = dm_device_id_from_name( dirname, &rest ) ) == DM_ERR_NO_DEVICE )
  {
    _REENT->_errno = ENOSYS;
    return NULL;
  }
  pdev = dm_list[ pos ].pdev;
  if( pdev->p_opendir_r == NULL )
  {
    _REENT->_errno = ENOSYS;
    return NULL;
  }
  if( ( data = pdev->p_opendir_r( _REENT, rest, dm_list[ pos ].pdata ) ) == NULL )
    return NULL;
  if( ( d = malloc( sizeof( DM_DIR ) ) ) == NULL )
  {
    _REENT->_errno = ENOMEM;
    return NULL;
  }
  d->devid = pos;
  d->userdata = data;
  return d;
}

// Read the next directory entry from the directory descriptor
struct dm_dirent* dm_readdir( DM_DIR *d )
{
  const DM_DEVICE *pdev;

  if( d->devid < 0 || d->devid >= dm_num_devs )
  {
    _REENT->_errno = EBADF;
    return NULL;
  }
  pdev = dm_list[ d->devid ].pdev;
  if( pdev->p_readdir_r == NULL )
  {
    _REENT->_errno = EBADF;
    return NULL;
  }
  return pdev->p_readdir_r( _REENT, d->userdata, dm_list[ d->devid ].pdata );
}

// Close a directory descriptor
int dm_closedir( DM_DIR *d )
{
  int res = -1;
  const DM_DEVICE *pdev;

  if( d->devid < 0 || d->devid >= dm_num_devs )
  {
    _REENT->_errno = EBADF;
    return -1;
  }
  pdev = dm_list[ d->devid ].pdev;
  if( pdev->p_closedir_r )
    res = pdev->p_closedir_r( _REENT, d->userdata, dm_list[ d->devid ].pdata );
  else
    _REENT->_errno = EBADF;
  free( d );
  return res;
}

const char* dm_getaddr( int fd )
{
  const DM_INSTANCE_DATA *pinst;

  // Find device, check write function
  pinst = dm_get_instance_at( DM_GET_DEVID( fd ) );
  if( !pinst || pinst->pdev->p_getaddr_r == NULL )
  {
    _REENT->_errno = ENOSYS;
    return NULL;
  }

  return pinst->pdev->p_getaddr_r( _REENT, DM_GET_FD( fd ), pinst->pdata );
}

