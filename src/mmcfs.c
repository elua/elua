// MMC filesystem implementation using FatFs
#include "mmcfs.h"
#include "type.h"
#include <string.h>
#include <errno.h>
#include "devman.h"
#include <stdio.h>
#include "ioctl.h"

#include "platform_conf.h"
#ifdef BUILD_MMCFS
#include "ff.h"
#include "diskio.h"
#include <fcntl.h>

#define MMCFS_MAX_FDS   4
static FIL mmcfs_fd_table[ MMCFS_MAX_FDS ];
static int mmcfs_num_fd;

// Data structures used by FatFs
static FATFS mmc_fs;
static FIL mmc_fileObject;
//static DIR mmc_dir;
//static FILINFO mmc_fileInfo;

#define PATH_BUF_SIZE   40
static char mmc_pathBuf[PATH_BUF_SIZE];

static int mmcfs_find_empty_fd( void )
{
  int i;

  for (i = 0; i < MMCFS_MAX_FDS; i ++)
    if (mmcfs_fd_table[i].fs == NULL)
      return i;
  return -1;
}

static int mmcfs_open_r( struct _reent *r, const char *path, int flags, int mode )
{
  int fd;
  int mmc_mode;

  if (mmcfs_num_fd == MMCFS_MAX_FDS)
  {
    r->_errno = ENFILE;
    return -1;
  }

  // Default to top directory if none given
  mmc_pathBuf[0] = 0;
  if (strchr(path, '/') == NULL)
    strcat(mmc_pathBuf, "/");
  strcat(mmc_pathBuf, path);

  // Scrub binary flag, if defined
#ifdef O_BINARY
  flags &= ~O_BINARY;
#endif

#if _FS_READONLY
  if ((flags & O_ACCMODE) != O_RDONLY)
  {
    r->_errno = EROFS;
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
    return -1;
  }
#endif  // _FS_READONLY

  // Open the file for reading
  if (f_open(&mmc_fileObject, mmc_pathBuf, mmc_mode) != FR_OK)
  {
    r->_errno = ENOENT;
    return -1;
  }

  if (mode & O_APPEND)
    mmc_fileObject.fptr = mmc_fileObject.fsize;
  fd = mmcfs_find_empty_fd();
  memcpy(mmcfs_fd_table + fd, &mmc_fileObject, sizeof(FIL));
  mmcfs_num_fd ++;
  return fd;
}

static int mmcfs_close_r( struct _reent *r, int fd )
{
  FIL* pFile = mmcfs_fd_table + fd;

  f_close( pFile );
  memset(pFile, 0, sizeof(FIL));
  mmcfs_num_fd --;
  return 0;
}

static _ssize_t mmcfs_write_r( struct _reent *r, int fd, const void* ptr, size_t len )
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

static _ssize_t mmcfs_read_r( struct _reent *r, int fd, void* ptr, size_t len )
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
static off_t mmcfs_lseek_r( struct _reent *r, int fd, off_t off, int whence )
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
static DIR mmc_dir;
static void* mmcfs_opendir_r( struct _reent *r, const char* dname )
{
  void* res = NULL;
  if( !dname || strlen( dname ) == 0 || ( strlen( dname ) == 1 && !strcmp( dname, "/" ) ) )
    res = f_opendir( &mmc_dir, "/" ) != FR_OK ? NULL : &mmc_dir; 
  return res;
}

// readdir
extern struct dm_dirent dm_shared_dirent;
extern char dm_shared_fname[ DM_MAX_FNAME_LENGTH + 1 ];
static struct dm_dirent* mmcfs_readdir_r( struct _reent *r, void *d )
{
  DIR *pdir = ( DIR* )d;
  FILINFO mmc_file_info;
  struct dm_dirent* pent = &dm_shared_dirent;
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
    if( ( mmc_file_info.fattrib & AM_DIR ) == 0 ) // if we have a file, exit loop
      break;
  }
#if _USE_LFN
  fn = *mmc_file_info.lfname ? mmc_file_info.lfname : mmc_file_info.fname;
#else
  fn = mmc_file_info.fname;
#endif
  strncpy( dm_shared_fname, fn, DM_MAX_FNAME_LENGTH );
  pent->fname = dm_shared_fname;
  pent->fsize = mmc_file_info.fsize;
  pent->ftime = mmc_file_info.ftime; 
  return pent;
}

// closedir
static int mmcfs_closedir_r( struct _reent *r, void *d )
{
  return 0;
}

// MMC device descriptor structure
static const DM_DEVICE mmcfs_device =
{
  "/mmc",
  mmcfs_open_r,         // open
  mmcfs_close_r,        // close
  mmcfs_write_r,        // write
  mmcfs_read_r,         // read
  mmcfs_lseek_r,        // lseek
  mmcfs_opendir_r,      // opendir
  mmcfs_readdir_r,      // readdir
  mmcfs_closedir_r,     // closedir
  NULL                  // getaddr
};

const DM_DEVICE* mmcfs_init()
{
  // Mount the MMC file system using logical disk 0
  if ( f_mount( 0, &mmc_fs ) != FR_OK )
    return NULL;

  return &mmcfs_device;
}

#else // #ifdef BUILD_MMCFS

const DM_DEVICE* mmcfs_init()
{
  return NULL;
}

#endif // BUILD_MMCFS
