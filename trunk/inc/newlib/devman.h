// Device manager interface for Newlib (stubs and additional IOCTL implementation)

#ifndef __DEVMAN_H__
#define __DEVMAN_H__

#include "type.h"
#include <reent.h>

// Maximum number of devices in the system
#define DM_MAX_DEVICES        16
#define DM_MAX_DEVICES_BITS   4

// Maximum number of a device name
#define DM_MAX_DEV_NAME       12

// Pack/unpack descriptions
// Even if a file descriptor is an 'int', newlib treats it as a short, so we need to stuff
// everything into 16 bits. Actually 15, since negative fd's are actually error indications
#define DM_MAKE_DESC( devid, fd ) ( ( ( devid ) << ( 15 - DM_MAX_DEVICES_BITS ) ) | ( fd ) )
#define DM_GET_DEVID( desc )      ( ( desc ) >> ( 15 - DM_MAX_DEVICES_BITS ) )
#define DM_GET_FD( desc )         ( ( desc ) & ( ( 1 << ( 15 - DM_MAX_DEVICES_BITS ) ) - 1 ) )

// STDIO file number
#define DM_STDIN_NUM              0
#define DM_STDOUT_NUM             1
#define DM_STDERR_NUM             2

// A device structure with pointers to all the device functions
typedef struct
{
  char name[ DM_MAX_DEV_NAME + 1 ];
  int ( *p_open_r )( struct _reent *r, const char *path, int flags, int mode );
  int ( *p_close_r )( struct _reent *r, int fd );
  _ssize_t ( *p_write_r ) ( struct _reent *r, int fd, const void *ptr, size_t len );
  _ssize_t ( *p_read_r )( struct _reent *r, int fd, void *ptr, size_t len );  
  int ( *p_ioctl_r )( struct _reent *r, int file, unsigned long request, void *ptr );  
} DM_DEVICE;

// Errors
#define DM_ERR_ALREADY_REGISTERED   (-1)
#define DM_ERR_NOT_REGISTERED       (-2)
#define DM_ERR_NO_SPACE             (-3)
#define DM_ERR_INVALID_NAME         (-4)
#define DM_OK                       (0)

// Add a device
int dm_register( DM_DEVICE *pdev );
// Unregister a device
int dm_unregister( const char* name );
// Get a device entry
const DM_DEVICE* dm_get_device_at( int idx );
// Returns the number of registered devices
int dm_get_num_devices();
// Initialize device manager
int dm_init();

#endif
