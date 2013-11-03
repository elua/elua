// Filesystem implementation
#include "swi.h"
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include "ioctl.h"
#include "semifs.h"
#include <stdint.h>
#include <fcntl.h>

#include "platform_conf.h"
#ifdef BUILD_SEMIFS

#define SEMIFS_MAX_FDS   4

// Custom Functions for mbed getting listings of files
#define RESERVED_FOR_USER_APPLICATIONS (0x100) /* 0x100 - 0x1ff */
#define USR_XFFIND (RESERVED_FOR_USER_APPLICATIONS + 0)    


typedef struct
{
  int handle;
  int pos;
} poslog;

static poslog semifs_fd_table[ SEMIFS_MAX_FDS ];
static int semifs_num_fd;


// Semihosting calls, some portions based on arm/syscalls.c from Newlib
static inline int
__semihost(int reason, void * arg)
{
  int value;
  asm volatile ("mov r0, %1; mov r1, %2; " AngelSWIInsn " %a3; mov %0, r0"
       : "=r" (value) /* Outputs */
       : "r" (reason), "r" (arg), "i" (AngelSWI) /* Inputs */
       : "r0", "r1", "r2", "r3", "ip", "lr", "memory", "cc"
                /* Clobbers r0 and r1, and lr if in supervisor mode */);
                /* Accordingly to page 13-77 of ARM DUI 0040D other registers
                   can also be clobbered.  Some memory positions may also be
                   changed by a system call, so they should not be kept in
                   registers. Note: we are assuming the manual is right and
                   Angel is respecting the APCS.  */
  return value;
}

static int semifs_find_empty_fd()
{
  int i;

  for (i = 0; i < SEMIFS_MAX_FDS; i ++)
    if (semifs_fd_table[ i ].handle == -1)
      return i;
  return -1;
}

static int semifs_open_r( struct _reent *r, const char *path, int flags, int mode, void *pdata )
{
  int aflags = 0, fd, fh;
  uint32_t args[3];

  if (semifs_num_fd == SEMIFS_MAX_FDS)
  {
    r->_errno = ENFILE;
    return -1;
  }

  if (flags & O_RDWR)
    aflags |= 2;

  if (flags & O_CREAT)
    aflags |= 4;

  if (flags & O_TRUNC)
    aflags |= 4;

  if (flags & O_APPEND)
  {
    aflags &= ~4;     /* Can't ask for w AND a; means just 'a'.  */
    aflags |= 8;
  }

  fd = semifs_find_empty_fd();
  semifs_num_fd ++;

  args[0] = ( uint32_t )path;
  args[1] = ( uint32_t )aflags;
  args[2] = ( uint32_t )strlen( path );
  fh =  __semihost(SYS_OPEN, args);
  
  if (fh >= 0)
  {
    semifs_fd_table[ fd ].handle = fh;
    semifs_fd_table[ fd ].pos = 0;
  }

  return fd;
}

static int semifs_close_r( struct _reent *r, int fd, void *pdata )
{
  int fh = semifs_fd_table[ fd ].handle;
   
  if( fd != SEMIFS_MAX_FDS )
    semifs_fd_table[ fd ].handle = -1;

  semifs_num_fd --;

  return __semihost(SYS_CLOSE, &fh);
}

static _ssize_t semifs_write_r( struct _reent *r, int fd, const void* ptr, size_t len, void *pdata )
{
  int fh = semifs_fd_table[ fd ].handle, x;
  uint32_t args[3];
  
  args[0] = (uint32_t)fh;
  args[1] = (uint32_t)ptr;
  args[2] = (uint32_t)len; 
  x = __semihost(SYS_WRITE, args);
  
  if (x == -1 || x == len)
  {
    r->_errno = EIO;
    return -1;
  }
  
  if ( fd != SEMIFS_MAX_FDS )
    semifs_fd_table[ fd ].pos += len - x;

  return len - x;
}

static _ssize_t semifs_read_r( struct _reent *r, int fd, void* ptr, size_t len, void *pdata )
{
  int fh = semifs_fd_table[ fd ].handle, x;
  uint32_t args[3];
  
  args[0] = (uint32_t)fh;
  args[1] = (uint32_t)ptr;
  args[2] = (uint32_t)len; 
  x = __semihost(SYS_READ, args);

  if (x < 0)
  {
    r->_errno = EIO;
    return -1;
  }

  if (fd != SEMIFS_MAX_FDS)
    semifs_fd_table[ fd ].pos += len - x;
  
  return len - x;
}

static off_t semifs_lseek_r( struct _reent *r, int fd, off_t off, int whence, void *pdata )
{
  int fh = semifs_fd_table[ fd ].handle, res;  
  uint32_t args[2];
  u32 newpos = 0;

  switch( whence )
  {
    case SEEK_CUR:
      // seek from current position
      if (fd == SEMIFS_MAX_FDS)
        return -1;
      off  += semifs_fd_table[ fd ].pos;
      whence = SEEK_SET;
      break;

    case SEEK_END:
      // seek from end of file
      args[ 0 ] = fh;
      off += __semihost(SYS_FLEN, &args);
      break;

    case SEEK_SET:
      break;

    default:
      return -1;
  }
  // Do absolute seek
  args[ 0 ] = (uint32_t)fh;
  args[ 1 ] = (uint32_t)off;
  res = __semihost(SYS_SEEK, args);

  if (fd != SEMIFS_MAX_FDS && res == 0)
    semifs_fd_table[ fd ].pos = off;

  /* This is expected to return the position in the file.  */
  return res == 0 ? off : -1;
}

static int xffind(const char *pattern, XFINFO *info)
{
    uint32_t param[4];

    param[0] = (uint32_t)pattern;
    param[1] = (uint32_t)strlen(pattern);
    param[2] = (uint32_t)info;
    param[3] = (uint32_t)sizeof(XFINFO);
    
    return __semihost(USR_XFFIND, param);
}

// opendir
char testpattern[] = "*";
SEARCHINFO semifs_dir;
static void* semifs_opendir_r( struct _reent *r, const char* dname, void *pdata )
{
  semifs_dir.file_info.fileID = 0;
  semifs_dir.pattern = testpattern;
  return ( void * )&semifs_dir;
}

// readdir
extern struct dm_dirent dm_shared_dirent;
extern char dm_shared_fname[ DM_MAX_FNAME_LENGTH + 1 ];
static struct dm_dirent* semifs_readdir_r( struct _reent *r, void *d, void *pdata )
{
  SEARCHINFO *dir = ( SEARCHINFO* )d;
  XFINFO *semifs_file_info = &dir->file_info;
  struct dm_dirent* pent = &dm_shared_dirent;
  int res;
  
  res = xffind(( char * )dir->pattern, semifs_file_info);
  if( res != 0 )
    return NULL;

  strncpy( dm_shared_fname, semifs_file_info->name, DM_MAX_FNAME_LENGTH );
  pent->fname = dm_shared_fname;
  pent->fsize = semifs_file_info->size;
  pent->ftime = 0; // need to convert from struct to UNIX time?!
  pent->flags = 0;
  return pent;
}

// closedir
static int semifs_closedir_r( struct _reent *r, void *d, void *pdata )
{
  return 0;
}

// Semihosting device descriptor structure
static const DM_DEVICE semifs_device =
{
  semifs_open_r,         // open
  semifs_close_r,        // close
  semifs_write_r,        // write
  semifs_read_r,         // read
  semifs_lseek_r,        // lseek
  semifs_opendir_r,      // opendir
  semifs_readdir_r,      // readdir
  semifs_closedir_r,     // closedir
  NULL,                  // getaddr
  NULL,                  // mkdir
  NULL,                  // unlink
  NULL,                  // rmdir                   
  NULL                   // rename
};

int semifs_init()
{
  int i;

  for (i = 0; i < SEMIFS_MAX_FDS; i ++)
    semifs_fd_table[i].handle = -1;

  return dm_register( "/semi", NULL, &semifs_device );
}

#else // #ifdef BUILD_SEMIFS

int semifs_init()
{
  return dm_register( NULL, NULL, NULL );
}

#endif // #ifdef BUILD_SEMIFS

