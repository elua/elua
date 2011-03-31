// A POSIX implementatoin of the os_io.h interface

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include "os_io.h"
#include "remotefs.h"
#include "eluarpc.h"

int os_open( const char *pathname, int flags, int mode )
{
  int realflags = 0;

  // Translate RFS flags to POSIX flags
  realflags = eluarpc_replace_flag( flags, RFS_OPEN_FLAG_APPEND, O_APPEND );
  realflags |= eluarpc_replace_flag( flags, RFS_OPEN_FLAG_CREAT, O_CREAT );
  realflags |= eluarpc_replace_flag( flags, RFS_OPEN_FLAG_EXCL, O_EXCL );
  realflags |= eluarpc_replace_flag( flags, RFS_OPEN_FLAG_TRUNC, O_TRUNC );
  realflags |= eluarpc_replace_flag( flags, RFS_OPEN_FLAG_SYNC, O_SYNC );
  realflags |= eluarpc_replace_flag( flags, RFS_OPEN_FLAG_RDONLY, O_RDONLY );
  realflags |= eluarpc_replace_flag( flags, RFS_OPEN_FLAG_WRONLY, O_WRONLY );
  realflags |= eluarpc_replace_flag( flags, RFS_OPEN_FLAG_RDWR, O_RDWR );
  return open( pathname, realflags, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH ); 
}

u32 os_open_sys_flags_to_rfs_flags( int sysflags )
{
  int rfsflags = 0;

  // Translate RFS flags to POSIX flags
  rfsflags = eluarpc_replace_flag( sysflags, O_APPEND, RFS_OPEN_FLAG_APPEND );
  rfsflags |= eluarpc_replace_flag( sysflags, O_CREAT, RFS_OPEN_FLAG_CREAT );
  rfsflags |= eluarpc_replace_flag( sysflags, O_EXCL, RFS_OPEN_FLAG_EXCL );
  rfsflags |= eluarpc_replace_flag( sysflags, O_TRUNC, RFS_OPEN_FLAG_TRUNC );
  rfsflags |= eluarpc_replace_flag( sysflags, O_SYNC, RFS_OPEN_FLAG_SYNC );
  rfsflags |= eluarpc_replace_flag( sysflags, O_RDONLY, RFS_OPEN_FLAG_RDONLY );
  rfsflags |= eluarpc_replace_flag( sysflags, O_WRONLY, RFS_OPEN_FLAG_WRONLY );
  rfsflags |= eluarpc_replace_flag( sysflags, O_RDWR, RFS_OPEN_FLAG_RDWR );
  return rfsflags;
}

s32 os_write( int fd, const void *buf, u32 count )
{
  return ( s32 )write( fd, buf, ( size_t )count );
}

s32 os_read( int fd, void *buf, u32 count )
{
  return ( s32 )read( fd, buf, ( size_t )count );
}

int os_close( int fd )
{
  return close( fd );
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
  return ( s32 )lseek( fd, ( off_t )offset, realwhence );
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
  struct stat res;

  if( stat( name, &res ) == -1 )
    return 0;
  else
    return S_ISDIR( res.st_mode );
}

u32 os_opendir( const char* name )
{
  if( name || strlen( name ) == 0 || ( strlen( name ) == 1 && !strcmp( name, "/" ) ) )
    return ( u32 )opendir( name );
  return 0;
}

void os_readdir( u32 d, const char **pname )
{
  struct dirent *ent;
  static char realname[ RFS_MAX_FNAME_SIZE + 1 ]; 

  while( 1 )
  {
    ent = readdir( ( DIR* )d );
    if( ent == NULL )
    {
      *pname = NULL;
      break;
    }
    if( ent->d_type & DT_REG )
    {
      realname[ 0 ] = realname[ RFS_MAX_FNAME_SIZE ] = '\0';
      strncpy( realname, ent->d_name, RFS_MAX_FNAME_SIZE );
      *pname = realname;
      break;
    }
  }
}

int os_closedir( u32 d )
{
  return closedir( ( DIR* )d );
}

