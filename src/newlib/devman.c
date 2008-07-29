// Device manager interface for Newlib (stubs and additional IOCTL implementation)

#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include "devman.h"
#include "genstd.h"

static DM_DEVICE dm_list[ DM_MAX_DEVICES ];           // list of devices
static int dm_num_devs;                               // number of devices

// Register a device
// Returns the index of the device in the device table
int dm_register( DM_DEVICE *pdev )
{
  int i;
  
  // First char of the name must be '/'
  if( pdev->name == NULL || *pdev->name == '\0' || *pdev->name != '/' || strlen( pdev->name ) > DM_MAX_DEV_NAME )
    return DM_ERR_INVALID_NAME;
  
  // Check if the device is not already registered
  for( i = 0; i < dm_num_devs; i ++ )
    if( !strcasecmp( pdev->name, dm_list[ i ].name ) )
      return DM_ERR_ALREADY_REGISTERED;
  
  // Check for space
  if( dm_num_devs == DM_MAX_DEVICES )
    return DM_ERR_NO_SPACE;
    
  // Register it now
  memcpy( dm_list + dm_num_devs, pdev, sizeof( DM_DEVICE ) );
  dm_num_devs ++;
  return dm_num_devs - 1;
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
    memmove( dm_list + i, dm_list + i + 1, ( dm_num_devs - i - 1 ) * sizeof( DM_DEVICE ) );
  dm_num_devs --;
  return DM_OK;
}

// Get a device entry
const DM_DEVICE* dm_get_device_at( int idx )
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
  dm_register( std_get_desc() );
  setbuf( stdout, NULL );
  return DM_OK;
}
