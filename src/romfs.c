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

static FD romfs_fd_table[ ROMFS_MAX_FDS ];
static int romfs_num_fd;

static int romfs_find_empty_fd()
{
  int i;
  
  for( i = 0; i < ROMFS_MAX_FDS; i ++ )
    if( romfs_fd_table[ i ].baseaddr == 0xFFFFFFFF &&
        romfs_fd_table[ i ].offset == 0xFFFFFFFF &&
        romfs_fd_table[ i ].size == 0xFFFFFFFF )
      return i;
  return -1;
}

static void romfs_close_fd( int fd )
{
  memset( romfs_fd_table + fd, 0xFF, sizeof( FD ) );
  romfs_fd_table[ fd ].flags = 0;
}

// Open the given file, returning one of FS_FILE_NOT_FOUND, FS_FILE_ALREADY_OPENED
// or FS_FILE_OK
static u8 romfs_open_file( const char* fname, u8 *pbase, FD* pfd )
{
  u32 i, j;
  char fsname[ DM_MAX_FNAME_LENGTH + 1 ];
  u32 fsize;
  
  // Look for the file
  i = 0;
  while( 1 )
  {
    if( pbase[ i ] == 0xFF )
      return FS_FILE_NOT_FOUND;
    // Read file name
    for( j = 0; j < DM_MAX_FNAME_LENGTH; j ++ )
    {
      fsname[ j ] = pbase[ i + j ];
      if( fsname[ j ] == 0 )
         break;
    }
    // ' i + j' now points at the '0' byte
    j = i + j + 1;
    // And read the size   
    fsize = pbase[ j ] + ( pbase[ j + 1 ] << 8 );
    fsize += ( pbase[ j + 2 ] << 16 ) + ( pbase[ j + 3 ] << 24 );
    j += 4;
    // Round to a multiple of ROMFS_ALIGN
    j = ( j + ROMFS_ALIGN - 1 ) & ~( ROMFS_ALIGN - 1 );
    if( !strncasecmp( fname, fsname, DM_MAX_FNAME_LENGTH ) )
    {
      // Found the file
      pfd->baseaddr = j;
      pfd->offset = 0;
      pfd->size = fsize;
      return FS_FILE_OK;
    }
    // Move to next file
    i = j + fsize;
  }
  return FS_FILE_NOT_FOUND;
}

static int romfs_open_r( struct _reent *r, const char *path, int flags, int mode, void *pdata )
{
  FD tempfs;
  int i;
  FSDATA *pfsdata = ( FSDATA* )pdata;
  
  if( romfs_num_fd == ROMFS_MAX_FDS )
  {
    r->_errno = ENFILE;
    return -1;
  }
  if( romfs_open_file( path, pfsdata->pbase, &tempfs ) != FS_FILE_OK )
  {
    r->_errno = ENOENT;
    return -1;
  }
  i = romfs_find_empty_fd();
  memcpy( romfs_fd_table + i, &tempfs, sizeof( FD ) );
  romfs_num_fd ++;
  return i;
}

static int romfs_close_r( struct _reent *r, int fd, void *pdata )
{
  romfs_close_fd( fd );
  romfs_num_fd --;
  return 0;
}

static _ssize_t romfs_write_r( struct _reent *r, int fd, const void* ptr, size_t len, void *pdata )
{
  r->_errno = EINVAL;
  return -1;
}

static _ssize_t romfs_read_r( struct _reent *r, int fd, void* ptr, size_t len, void *pdata )
{
  FD* pfd = romfs_fd_table + fd;
  long actlen = fsmin( len, pfd->size - pfd->offset );
  FSDATA *pfsdata = ( FSDATA* )pdata;
  
  memcpy( ptr, pfsdata->pbase + pfd->offset + pfd->baseaddr, actlen );
  pfd->offset += actlen;
  return actlen;
}

// lseek
static off_t romfs_lseek_r( struct _reent *r, int fd, off_t off, int whence, void *pdata )
{
  FD* pfd = romfs_fd_table + fd;
  u32 newpos = 0;
  
  switch( whence )
  {
    case SEEK_SET:
      newpos = off;
      break;
      
    case SEEK_CUR:
      newpos = pfd->offset + off;
      break;
      
    case SEEK_END:
      newpos = pfd->size + off;
      break;
      
    default:
      return -1;
  }    
  if( newpos > pfd->size )
    return -1;
  pfd->offset = newpos;
  return newpos;
}

// Directory operations
static u32 romfs_dir_data = 0;

// opendir
static void* romfs_opendir_r( struct _reent *r, const char* dname, void *pdata )
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
static struct dm_dirent* romfs_readdir_r( struct _reent *r, void *d, void *pdata )
{
  u32 off = *( u32* )d;
  struct dm_dirent *pent = &dm_shared_dirent;
  unsigned j = 0;
  FSDATA *pfsdata = ( FSDATA* )pdata;
  u8 *pbase = pfsdata->pbase;
  
  if( pbase[ off ] == 0xFF )
    return NULL;
  while( ( dm_shared_fname[ j ++ ] = pbase[ off ++ ] ) != '\0' );
  pent->fname = dm_shared_fname;
  pent->fsize = pbase[ off ] + ( pbase[ off + 1 ] << 8 );
  pent->fsize += ( pbase[ off + 2 ] << 16 ) + ( pbase[ off + 3 ] << 24 );
  pent->ftime = 0;
  off += 4;
  off = ( off + ROMFS_ALIGN - 1 ) & ~( ROMFS_ALIGN - 1 );
  *( u32* )d = off + pent->fsize;
  return pent;
}

// closedir
static int romfs_closedir_r( struct _reent *r, void *d, void *pdata )
{
  *( u32* )d = 0;
  return 0;
}

// getaddr
static const char* romfs_getaddr_r( struct _reent *r, int fd, void *pdata )
{
  FD* pfd = romfs_fd_table + fd;
  FSDATA *pfsdata = ( FSDATA* )pdata;

  if( pfsdata->flags & ROMFS_FS_FLAG_DIRECT )
    return ( const char* )pfsdata->pbase + pfd->baseaddr;
  else
    return NULL;
}

// Our ROMFS device descriptor structure
static const DM_DEVICE romfs_device = 
{
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

// ROMFS descriptor
static const FSDATA romfs_fsdata =
{
  ( u8* )romfiles_fs,
  ROMFS_FS_FLAG_DIRECT
};

int romfs_init()
{
  unsigned i;

  for( i = 0; i < ROMFS_MAX_FDS; i ++ )
  {
    memset( romfs_fd_table + i, 0xFF, sizeof( FD ) );
    romfs_fd_table[ i ].flags = 0;
  }
  return dm_register( "/rom", ( void* )&romfs_fsdata, &romfs_device );
}

#else // #ifdef BUILD_ROMFS

int romfs_init()
{
  return dm_register( NULL, NULL, NULL );
}

#endif // #ifdef BUILD_ROMFS

