// Device manager interface for Newlib (stubs and additional IOCTL implementation)

#ifndef __DEVMAN_H__
#define __DEVMAN_H__

#include "type.h"
#include <reent.h>
#include <unistd.h>
#include <sys/types.h>

// Maximum number of devices in the system
#define DM_MAX_DEVICES        16
#define DM_MAX_DEVICES_BITS   4

// Maximum number of a device name
#define DM_MAX_DEV_NAME       12

// GLOBAL maximum file length (on ALL supported filesystem)
//TH: Overwrite max filename length with config from mmcfs 
#ifdef MMCFS_MAX_LFN
#define DM_MAX_FNAME_LENGTH MMCFS_MAX_LFN
#else
#define DM_MAX_FNAME_LENGTH   30
#endif 

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

// Directory entry flags
#define DM_DIRENT_FLAG_DIR        1

// Our platform independent "dirent" structure (for opendir/readdir)
struct dm_dirent {
  u32 fsize;
  const char *fname;
  u32 ftime;
  u8 flags;
};
typedef struct {
  u8 devid;
  void *userdata;
} DM_DIR;

#define DM_DIRENT_IS_DIR( ent )   ( ( ( ent )->flags & DM_DIRENT_FLAG_DIR ) != 0 )

// A device structure with pointers to all the device functions
typedef int mkdir_mode_t;

typedef struct
{
  int ( *p_open_r )( struct _reent *r, const char *path, int flags, int mode, void *pdata );
  int ( *p_close_r )( struct _reent *r, int fd, void *pdata );
  _ssize_t ( *p_write_r ) ( struct _reent *r, int fd, const void *ptr, size_t len, void *pdata );
  _ssize_t ( *p_read_r )( struct _reent *r, int fd, void *ptr, size_t len, void *pdata );
  off_t ( *p_lseek_r )( struct _reent *r, int fd, off_t off, int whence, void *pdata );
  void* ( *p_opendir_r )( struct _reent *r, const char* name, void *pdata );
  struct dm_dirent* ( *p_readdir_r )( struct _reent *r, void *dir, void *pdata );
  int ( *p_closedir_r )( struct _reent *r, void* dir, void *pdata );
  const char* ( *p_getaddr_r )( struct _reent *r, int fd, void *pdata );
  int ( *p_mkdir_r )( struct _reent *r, const char *pathname, mkdir_mode_t mode, void *pdata );
  int ( *p_unlink_r )( struct _reent *r, const char *fname, void *pdata );
  int ( *p_rmdir_r )( struct _reent *r, const char *fname, void *pdata );
  int ( *p_rename_r )( struct _reent *r, const char *oldname, const char *newname, void *pdata );
} DM_DEVICE;

// Additional registration data for each FS (per FS instance)
// This contains the name and additional data that the FS wants to save inside DM
// This data can be received later by the FS
// With this method, one can implement more than one instance of the same FS.
// For example, multiple ROM file systems can be implemented by calling
// "dm_register" multiple times with different DM_INSTANCE_DATA structures.
// The name will be different and "pdata" can point to a structure uniquely
// identifying this FS (pointer to its read function, start address, end address...)

typedef struct {
  const char *name;
  void *pdata;
  const DM_DEVICE *pdev;
} DM_INSTANCE_DATA;

// Errors
#define DM_ERR_ALREADY_REGISTERED   (-1)
#define DM_ERR_NOT_REGISTERED       (-2)
#define DM_ERR_NO_SPACE             (-3)
#define DM_ERR_INVALID_NAME         (-4)
#define DM_ERR_NO_DEVICE            (-5)
#define DM_ERR_INVALID_OPS          (-6)
#define DM_ERR_INIT                 (-7)
#define DM_OK                       (0)

// Add a device
int dm_register( const char *name, void *pdata, const DM_DEVICE* pdev );
// Unregister a device
int dm_unregister( const char* name );
// Get a device entry
const DM_DEVICE* dm_get_device_at( int idx );
// Get an instance
const DM_INSTANCE_DATA* dm_get_instance_at( int idx );
// Returns the number of registered devices
int dm_get_num_devices(void);
// Initialize device manager
int dm_init(void);

// DM specific functions (uniform over all the installed filesystems)
DM_DIR *dm_opendir( const char* dirname );
struct dm_dirent* dm_readdir( DM_DIR *d );
int dm_closedir( DM_DIR *d );
const char* dm_getaddr( int fd );

#endif

