// Filesystem implementation
#include "romfs.h"
#include "type.h"
#include <string.h>
#include <errno.h>
#include "devman.h"
#include "romfiles.h"
#include <stdio.h>
#include "ioctl.h"

#include "platform_conf.h"
#ifdef BUILD_ROMFS

#define ROMFS_MAX_FDS   4
#define ROMFS_ALIGN     4
#define fsmin( x , y ) ( ( x ) < ( y ) ? ( x ) : ( y ) )

static FS romfs_fd_table[ ROMFS_MAX_FDS ];
static int romfs_num_fd;

static u8 romfs_read( u32 addr )
{
  return romfiles_fs[ addr ];
}

static int romfs_find_empty_fd()
{
  int i;
  
  for( i = 0; i < ROMFS_MAX_FDS; i ++ )
    if( romfs_fd_table[ i ].p_read_func == NULL )
      return i;
  return -1;
}

static void romfs_close_fd( int fd )
{
  memset( romfs_fd_table + fd, 0, sizeof( FS ) );
}

// Open the given file, returning one of FS_FILE_NOT_FOUND, FS_FILE_ALREADY_OPENED
// or FS_FILE_OK
u8 romfs_open_file( const char* fname, p_read_fs_byte p_read_func, FS* pfs )
{
  u32 i, j;
  char fsname[ DM_MAX_FNAME_LENGTH + 1 ];
  u32 fsize;
  
  // Look for the file
  i = 0;
  while( 1 )
  {
    // Read file name
    for( j = 0; j < DM_MAX_FNAME_LENGTH; j ++ )
    {
      fsname[ j ] = p_read_func( i + j );
      if( fsname[ j ] == 0 )
      {
        if( j == 0 )
          return FS_FILE_NOT_FOUND;
        else
          break;
      }
    }
    // ' i + j' now points at the '0' byte
    j = i + j + 1;
    // And read the size   
    fsize = p_read_func( j ) + ( p_read_func( j + 1 ) << 8 );
    fsize += ( p_read_func( j + 2 ) << 16 ) + ( p_read_func( j + 3 ) << 24 );
    j += 4;
    // Round to a multiple of ROMFS_ALIGN
    j = ( j + ROMFS_ALIGN - 1 ) & ~( ROMFS_ALIGN - 1 );
    if( !strncasecmp( fname, fsname, DM_MAX_FNAME_LENGTH ) )
    {
      // Found the file
      pfs->baseaddr = j;
      pfs->offset = 0;
      pfs->size = fsize;
      pfs->p_read_func = p_read_func;   
      return FS_FILE_OK;
    }
    // Move to next file
    i = j + fsize;
  }
  return FS_FILE_NOT_FOUND;
}

static int romfs_open_r( struct _reent *r, const char *path, int flags, int mode )
{
  FS tempfs;
  int i;
  
  if( romfs_num_fd == ROMFS_MAX_FDS )
  {
    r->_errno = ENFILE;
    return -1;
  }
  if( romfs_open_file( path, romfs_read, &tempfs ) != FS_FILE_OK )
  {
    r->_errno = ENOENT;
    return -1;
  }
  i = romfs_find_empty_fd();
  memcpy( romfs_fd_table + i, &tempfs, sizeof( FS ) );
  romfs_num_fd ++;
  return i;
}

static int romfs_close_r( struct _reent *r, int fd )
{
  romfs_close_fd( fd );
  romfs_num_fd --;
  return 0;
}

static _ssize_t romfs_write_r( struct _reent *r, int fd, const void* ptr, size_t len )
{
  r->_errno = EINVAL;
  return -1;
}

static _ssize_t romfs_read_r( struct _reent *r, int fd, void* ptr, size_t len )
{
  FS* pfs = romfs_fd_table + fd; 
  long actlen = fsmin( len, pfs->size - pfs->offset );
  
  memcpy( ptr, romfiles_fs + pfs->offset + pfs->baseaddr, actlen );
  pfs->offset += actlen;
  return actlen;
}

// lseek
static off_t romfs_lseek_r( struct _reent *r, int fd, off_t off, int whence )
{
  FS* pfs = romfs_fd_table + fd;   
  u32 newpos = 0;
  
  switch( whence )
  {
    case SEEK_SET:
      newpos = off;
      break;
      
    case SEEK_CUR:
      newpos = pfs->offset + off;
      break;
      
    case SEEK_END:
      newpos = pfs->size + off;
      break;
      
    default:
      return -1;
  }    
  if( newpos > pfs->size )
    return -1;
  pfs->offset = newpos;      
  return newpos;
}

// Directory operations
static u32 romfs_dir_data = 0;

// opendir
static void* romfs_opendir_r( struct _reent *r, const char* dname )
{
  if( !dname || strlen( dname ) == 0 || ( strlen( dname ) == 1 && !strcmp( dname, "/" ) ) )
  {
    romfs_dir_data = 0;
    return &romfs_dir_data;
  }
  return NULL;
}

// readdir
extern struct dm_dirent dm_shared_dirent;
extern char dm_shared_fname[ DM_MAX_FNAME_LENGTH + 1 ];
static struct dm_dirent* romfs_readdir_r( struct _reent *r, void *d )
{
  u32 off = *( u32* )d;
  struct dm_dirent *pent = &dm_shared_dirent;
  unsigned j = 0;
  
  if( romfs_read( off ) == 0 )
    return NULL;
  while( ( dm_shared_fname[ j ++ ] = romfs_read( off ++ ) ) != '\0' );
  pent->fname = dm_shared_fname;
  pent->fsize = romfs_read( off ) + ( romfs_read( off + 1 ) << 8 );
  pent->fsize += ( romfs_read( off + 2 ) << 16 ) + ( romfs_read( off + 3 ) << 24 );
  pent->ftime = 0;
  off += 4;
  off = ( off + ROMFS_ALIGN - 1 ) & ~( ROMFS_ALIGN - 1 );
  *( u32* )d = off + pent->fsize;
  return pent;
}

// closedir
static int romfs_closedir_r( struct _reent *r, void *d )
{
  *( u32* )d = 0;
  return 0;
}

// getaddr
static const char* romfs_getaddr_r( struct _reent *r, int fd )
{
  FS* pfs = romfs_fd_table + fd;

  return ( const char* )romfiles_fs + pfs->baseaddr;
}

// Our ROMFS device descriptor structure
static const DM_DEVICE romfs_device = 
{
  "/rom",
  romfs_open_r,         // open
  romfs_close_r,        // close
  romfs_write_r,        // write
  romfs_read_r,         // read
  romfs_lseek_r,        // lseek
  romfs_opendir_r,      // opendir
  romfs_readdir_r,      // readdir
  romfs_closedir_r,     // closedir
  romfs_getaddr_r,      // getaddr
};

const DM_DEVICE* romfs_init()
{
  return &romfs_device;
}

#else // #ifdef BUILD_ROMFS

const DM_DEVICE* romfs_init()
{
  return NULL;
}

#endif // #ifdef BUILD_ROMFS

