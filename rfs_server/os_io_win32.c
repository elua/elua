// A Win32 implementation of the os_io.h interface

#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include <windows.h>
#include "os_io.h"
#include "remotefs.h"

int os_open( const char *pathname, int flags, int mode )
{
  int realflags = 0;

  // Translate RFS flags to POSIX flags                                                                                                     
  realflags = remotefs_replace_flag( flags, RFS_OPEN_FLAG_APPEND, _O_APPEND );
  realflags |= remotefs_replace_flag( flags, RFS_OPEN_FLAG_CREAT, _O_CREAT );
  realflags |= remotefs_replace_flag( flags, RFS_OPEN_FLAG_EXCL, _O_EXCL );
  realflags |= remotefs_replace_flag( flags, RFS_OPEN_FLAG_TRUNC, _O_TRUNC );
  realflags |= remotefs_replace_flag( flags, RFS_OPEN_FLAG_RDONLY, _O_RDONLY );
  realflags |= remotefs_replace_flag( flags, RFS_OPEN_FLAG_WRONLY, _O_WRONLY );
  realflags |= remotefs_replace_flag( flags, RFS_OPEN_FLAG_RDWR, _O_RDWR );
  return _sopen( pathname, realflags, _SH_DENYNO, _S_IREAD | _S_IWRITE ); 
}

u32 os_open_sys_flags_to_rfs_flags( int sysflags )
{
  int rfsflags = 0;

  // Translate RFS flags to POSIX flags
  rfsflags = remotefs_replace_flag( sysflags, _O_APPEND, RFS_OPEN_FLAG_APPEND );
  rfsflags |= remotefs_replace_flag( sysflags, _O_CREAT, RFS_OPEN_FLAG_CREAT );
  rfsflags |= remotefs_replace_flag( sysflags, _O_EXCL, RFS_OPEN_FLAG_EXCL );
  rfsflags |= remotefs_replace_flag( sysflags, _O_TRUNC, RFS_OPEN_FLAG_TRUNC );
  rfsflags |= remotefs_replace_flag( sysflags, _O_RDONLY, RFS_OPEN_FLAG_RDONLY );
  rfsflags |= remotefs_replace_flag( sysflags, _O_WRONLY, RFS_OPEN_FLAG_WRONLY );
  rfsflags |= remotefs_replace_flag( sysflags, _O_RDWR, RFS_OPEN_FLAG_RDWR );
  return rfsflags;
}

s32 os_write( int fd, const void *buf, u32 count )
{
  return ( s32 )_write( fd, buf, ( unsigned int )count );
}

s32 os_read( int fd, void *buf, u32 count )
{
  return ( s32 )_read( fd, buf, ( unsigned int )count );
}

int os_close( int fd )
{
  return _close( fd );
}

s32 os_lseek( int fd, s32 offset, int whence )
{
  int realwhence = 0;

  // Translate RFS whence to POSIX whence
  switch( whence )
  {
    case RFS_LSEEK_SET:
      realwhence = SEEK_SET;
      break;

   case RFS_LSEEK_CUR:
     realwhence = SEEK_CUR;
     break;

   case RFS_LSEEK_END:
     realwhence = SEEK_END;
     break;

   default:
     return -1;
  }
  return ( s32 )_lseek( fd, ( long )offset, realwhence );
}

u32 os_lseek_sys_whence_to_rfs_whence( int syswhence )
{
  switch( syswhence )
  {
    case SEEK_SET:
      return RFS_LSEEK_SET;

    case SEEK_CUR:
      return RFS_LSEEK_CUR;

    case SEEK_END:
      return RFS_LSEEK_END;
  }
  return 0;
}

int os_isdir( const char* name )
{
  DWORD attrs = GetFileAttributes( ( LPCTSTR )name );
  
  if( attrs == INVALID_FILE_ATTRIBUTES )
    return 0;
  else 
    return attrs & FILE_ATTRIBUTE_DIRECTORY;
}
