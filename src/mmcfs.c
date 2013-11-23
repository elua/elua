// MMC filesystem implementation using FatFs
#include "mmcfs.h"
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include "ioctl.h"
#include <sys/types.h>

#include "platform_conf.h"
#ifdef BUILD_MMCFS
#include "ff.h"
#include "diskio.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>

#define MMCFS_MAX_FDS   4
static FIL mmcfs_fd_table[ MMCFS_MAX_FDS ];
static int mmcfs_num_fd;

extern void elua_mmc_init( void );

#ifndef MMCFS_NUM_CARDS
#define NUM_CARDS             1
#else
#define NUM_CARDS             MMCFS_NUM_CARDS
#endif

// Data structures used by FatFs
static FATFS mmc_fs[ NUM_CARDS ];
static FIL mmc_fileObject;
//static DIR mmc_dir;
//static FILINFO mmc_fileInfo;
typedef struct
{
  DIR *dir;
  struct dm_dirent *pdm;
} MMCFS_DIRENT_DATA;

static int mmcfs_find_empty_fd( void )
{
  int i;

  for (i = 0; i < MMCFS_MAX_FDS; i ++)
    if (mmcfs_fd_table[i].fs == NULL)
      return i;
  return -1;
}

static int mmcfs_open_r( struct _reent *r, const char *path, int flags, int mode, void *pdata )
{
  int fd;
  int mmc_mode;
  char *mmc_pathBuf;
  int drv_num = *( int* )pdata;

  if (mmcfs_num_fd == MMCFS_MAX_FDS)
  {
    r->_errno = ENFILE;
    return -1;
  }

  // Default to top directory if none given
  if (strchr(path, '/') == NULL)
    asprintf( &mmc_pathBuf, "%d:/%s", drv_num, path );
  else
    asprintf( &mmc_pathBuf, "%d:%s", drv_num, path );
  if( mmc_pathBuf == NULL )
  {
    r->_errno = ENOMEM;
    return -1;
  }

  // Scrub binary flag, if defined
#ifdef O_BINARY
  flags &= ~O_BINARY;
#endif

#if _FS_READONLY
  if ((flags & O_ACCMODE) != O_RDONLY)
  {
    r->_errno = EROFS;
    free( mmc_pathBuf );
    return -1;
  }
  mmc_mode = FA_OPEN_EXISTING & FA_READ;
#else
  // Translate fcntl.h mode to FatFs mode (by jcwren@jcwren.com)
  if (((flags & (O_CREAT | O_TRUNC)) == (O_CREAT | O_TRUNC)) && (flags & (O_RDWR | O_WRONLY)))
    mmc_mode = FA_CREATE_ALWAYS;
  else if ((flags & (O_CREAT | O_EXCL)) == (O_CREAT | O_EXCL))
    mmc_mode = FA_OPEN_EXISTING;
  else if ((flags & O_CREAT) == O_CREAT)
    mmc_mode = FA_OPEN_ALWAYS;
  else if ((flags == O_RDONLY) || (flags == O_WRONLY) || (flags == O_RDWR))
    mmc_mode = FA_OPEN_EXISTING;
  else
  {
    r->_errno = EINVAL;
    free( mmc_pathBuf );
    return -1;
  }

  if ((flags & O_ACCMODE) == O_RDONLY)
    mmc_mode |= FA_READ;
  else if ((flags & O_ACCMODE) == O_WRONLY)
    mmc_mode |= FA_WRITE;
  else if ((flags & O_ACCMODE) == O_RDWR)
    mmc_mode |= (FA_READ | FA_WRITE);
  else
  {
    r->_errno = EINVAL;
    free( mmc_pathBuf );
    return -1;
  }
#endif  // _FS_READONLY

  // Open the file for reading
  if (f_open(&mmc_fileObject, mmc_pathBuf, mmc_mode) != FR_OK)
  {
    r->_errno = ENOENT;
    free( mmc_pathBuf );
    return -1;
  }

  if (mode & O_APPEND)
    mmc_fileObject.fptr = mmc_fileObject.fsize;
  fd = mmcfs_find_empty_fd();
  memcpy(mmcfs_fd_table + fd, &mmc_fileObject, sizeof(FIL));
  mmcfs_num_fd ++;
  free( mmc_pathBuf );
  return fd;
}

static int mmcfs_close_r( struct _reent *r, int fd, void *pdata )
{
  FIL* pFile = mmcfs_fd_table + fd;

  f_close( pFile );
  memset(pFile, 0, sizeof(FIL));
  mmcfs_num_fd --;
  return 0;
}

static _ssize_t mmcfs_write_r( struct _reent *r, int fd, const void* ptr, size_t len, void *pdata )
{
#if _FS_READONLY
  {
    r->_errno = EIO;
    return -1;
  }
#else
  UINT bytesWritten;

  if (f_write(mmcfs_fd_table + fd, ptr, len, &bytesWritten) != FR_OK)
  {
    r->_errno = EIO;
    return -1;
  }

  return (_ssize_t) bytesWritten;
#endif // _FS_READONLY
}

static _ssize_t mmcfs_read_r( struct _reent *r, int fd, void* ptr, size_t len, void *pdata )
{
  UINT bytesRead;

  if (f_read(mmcfs_fd_table + fd, ptr, len, &bytesRead) != FR_OK)
  {
    r->_errno = EIO;
    return -1;
  }

  return (_ssize_t) bytesRead;
}

// lseek
static off_t mmcfs_lseek_r( struct _reent *r, int fd, off_t off, int whence, void *pdata )
{
  FIL* pFile = mmcfs_fd_table + fd;
  u32 newpos = 0;

  switch( whence )
  {
    case SEEK_SET:
      // seek from beginning of file
      newpos = off;
      break;

    case SEEK_CUR:
      // seek from current position
      newpos = pFile->fptr + off;
      break;

    case SEEK_END:
      // seek from end of file
      newpos = pFile->fsize + off;
      break;

    default:
      return -1;
  }
  if (f_lseek (pFile, newpos) != FR_OK)
    return -1;
  return newpos;
}

// opendir
static void* mmcfs_opendir_r( struct _reent *r, const char* dname, void *pdata )
{
  void* res = NULL;
  MMCFS_DIRENT_DATA *pd = ( MMCFS_DIRENT_DATA* )malloc( sizeof( MMCFS_DIRENT_DATA ) );
  int drv_num = *( int* )pdata;
  char *pname = NULL;

  if( !pd )
    return NULL;
  memset( pd, 0, sizeof( MMCFS_DIRENT_DATA ) );
  if( ( pd->dir = ( DIR* )malloc( sizeof( DIR ) ) ) == NULL )
    goto out;
  if( ( pd->pdm = ( struct dm_dirent* )malloc( sizeof( struct dm_dirent ) ) ) == NULL )
    goto out;
  if( !dname || strlen( dname ) == 0 )
    asprintf( &pname, "%d:/", drv_num );
  else
    asprintf( &pname, "%d:%s", drv_num, dname );
  res = f_opendir( pd->dir, pname ) != FR_OK ? NULL : pd;
out:    
  if( res == NULL )
  {
    if( pd->dir )
      free( pd->dir );
    if( pd->pdm )
      free( pd->pdm );
    free( pd );
  }
  if( pname )
    free( pname );
  return res;
}

// readdir
extern char dm_shared_fname[ DM_MAX_FNAME_LENGTH + 1 ];
static struct dm_dirent* mmcfs_readdir_r( struct _reent *r, void *d, void *pdata )
{
  MMCFS_DIRENT_DATA *pd = ( MMCFS_DIRENT_DATA* )d;
  DIR *pdir = pd->dir;
  FILINFO mmc_file_info;
  struct dm_dirent* pent = pd->pdm;
  char *fn;
#if _USE_LFN
  static char lfn[_MAX_LFN * (_DF1S ? 2 : 1) + 1];
  mmc_file_info.lfname = lfn;
  mmc_file_info.lfsize = sizeof(lfn);
#endif
  
  while( 1 ) // loop until we get a file, error, or end of directory
  {
    if( f_readdir( pdir, &mmc_file_info ) != FR_OK ) // return NULL on read error
      return NULL;
    if( mmc_file_info.fname[ 0 ] == '\0' ) // return NULL when listing is done
      return NULL;
    break;
  }
#if _USE_LFN
  fn = *mmc_file_info.lfname ? mmc_file_info.lfname : mmc_file_info.fname;
#else
  fn = mmc_file_info.fname;
#endif
  if( ( mmc_file_info.fattrib & AM_DIR ) != 0 ) // if we have a file, exit loop
    pent->flags = DM_DIRENT_FLAG_DIR;
  else
    pent->flags = 0;
  strncpy( dm_shared_fname, fn, DM_MAX_FNAME_LENGTH );
  pent->fname = dm_shared_fname;
  pent->fsize = mmc_file_info.fsize;
  pent->ftime = mmc_file_info.ftime; 
  return pent;
}

// closedir
static int mmcfs_closedir_r( struct _reent *r, void *d, void *pdata )
{
  MMCFS_DIRENT_DATA *pd = ( MMCFS_DIRENT_DATA* )d;

  free( pd->dir );
  free( pd->pdm );
  free( pd );
  return 0;
}

static int mmcfs_mkdir_r( struct _reent *r, const char *name, mkdir_mode_t mode, void *pdata )
{
  return f_mkdir( name );
}

static int mmcfs_unlink_r( struct _reent *r, const char *fname, void *pdata )
{
  return f_unlink( fname );
}

static int mmcfs_rename_r( struct _reent *r, const char *oldname, const char *newname, void *pdata )
{
  return f_rename( oldname, newname );
}

// MMC device descriptor structure
static const DM_DEVICE mmcfs_device =
{
  mmcfs_open_r,         // open
  mmcfs_close_r,        // close
  mmcfs_write_r,        // write
  mmcfs_read_r,         // read
  mmcfs_lseek_r,        // lseek
  mmcfs_opendir_r,      // opendir
  mmcfs_readdir_r,      // readdir
  mmcfs_closedir_r,     // closedir
  NULL,                 // getaddr
  mmcfs_mkdir_r,        // mkdir
  mmcfs_unlink_r,       // unlink
  mmcfs_unlink_r,       // rmdir
  mmcfs_rename_r        // rename
};

int mmcfs_init()
{
  elua_mmc_init();
#if NUM_CARDS == 1
  static int cid = 0;
  // A single MMCFS
  if ( f_mount( 0, mmc_fs ) != FR_OK )
    return DM_ERR_INIT;
  return dm_register( "/mmc", &cid, &mmcfs_device );
#else // #if NUM_CARDS == 1
  int i;
  static char names[ NUM_CARDS ][ 6 ];
  static int ids[ NUM_CARDS ];

  // [TODO] add more error checking!
  for( i = 0; i < NUM_CARDS; i ++ )
    if( f_mount( i, mmc_fs + i ) == FR_OK )
    {
      ids[ i ] = i;
      sprintf( names[ i ], "/mmc%d", i );
      dm_register( names[ i ], ids + i, &mmcfs_device );
    }
  return DM_OK;
#endif // #if NUM_CARDS == 1
}

#else // #ifdef BUILD_MMCFS

int mmcfs_init()
{
  return dm_register( NULL, NULL, NULL );
}

#endif // BUILD_MMCFS
