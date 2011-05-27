// Remote FS IO adapter for eLua

#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include "os_io.h"
#include "remotefs.h"
#include "eluarpc.h"

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

