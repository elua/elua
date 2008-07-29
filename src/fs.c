// Filesystem implementation
#include "fs.h"
#include "type.h"
#include <string.h>
#include <errno.h>
#include "devman.h"
#include "luatest.h"
#include <stdio.h>

#define ROMFS_MAX_FDS   4
#define fsmin( x , y ) ( ( x ) < ( y ) ? ( x ) : ( y ) )

static FS romfs_fd_table[ ROMFS_MAX_FDS ];
static int romfs_num_fd;

static u8 fs_read( u16 addr )
{
  return luatest_fs[ addr ];
}

static int fs_find_empty_fd()
{
  int i;
  
  for( i = 0; i < ROMFS_MAX_FDS; i ++ )
    if( romfs_fd_table[ i ].p_read_func == NULL )
      return i;
  return -1;
}

static void fs_close_fd( int fd )
{
  memset( romfs_fd_table + fd, 0, sizeof( FS ) );
}

// Open the given file, returning one of FS_FILE_NOT_FOUND, FS_FILE_ALREADY_OPENED
// or FS_FILE_OK
u8 fs_open_file( const char* fname, p_read_fs_byte p_read_func, FS* pfs )
{
  u16 i, j;
  char fsname[ MAX_FNAME_LENGTH + 1 ];
  int fsize;
  
  // Look for the file
  i = 0;
  while( 1 )
  {
    // Read file name
    for( j = 0; j < MAX_FNAME_LENGTH; j ++ )
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
    if( !strncmp( fname, fsname, MAX_FNAME_LENGTH ) )
    {
      // Found the file
      pfs->baseaddr = j + 2;
      pfs->offset = 0;
      pfs->size = fsize;
      pfs->p_read_func = p_read_func;   
      return FS_FILE_OK;
    }
    // Move to next file
    i = j + 2 + fsize;
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
  if( fs_open_file( path, fs_read, &tempfs ) != FS_FILE_OK )
  {
    r->_errno = ENOENT;
    return -1;
  }
  i = fs_find_empty_fd();
  memcpy( romfs_fd_table + i, &tempfs, sizeof( FS ) );
  romfs_num_fd ++;
  return i;
}

static int romfs_close_r( struct _reent *r, int fd )
{
  fs_close_fd( fd );
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
  
  memcpy( ptr, luatest_fs + pfs->offset + pfs->baseaddr, actlen );
  pfs->offset += actlen;
  return actlen;
}

// Our UART device descriptor structure
static DM_DEVICE romfs_device = 
{
  "/rom",
  romfs_open_r,  
  romfs_close_r, 
  romfs_write_r,
  romfs_read_r,
  NULL                              // No "ioctl" on ROMFS
};

DM_DEVICE* fs_init()
{
  return &romfs_device;
}
