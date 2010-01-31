// OS interface for file I/O manipulation

#ifndef __OS_IO_H__
#define __OS_IO_H__

#include "type.h"

#ifdef WIN32_BUILD
#define PLATFORM_PATH_SEPARATOR     '\\'
#define PLATFORM_MAX_FNAME_LEN      2048
#else
#define PLATFORM_PATH_SEPARATOR     '/'
#define PLATFORM_MAX_FNAME_LEN      2048
#endif

int os_open( const char *pathname, int flags, int mode );
u32 os_open_sys_flags_to_rfs_flags( int sysflags );
s32 os_write( int fd, const void *buf, u32 count );
s32 os_read( int fd, void *buf, u32 count );
int os_close( int fd );
s32 os_lseek( int fd, s32 offset, int whence );
u32 os_lseek_sys_whence_to_rfs_whence( int syswhence );
int os_isdir( const char *name );
u32 os_opendir( const char* name );
void os_readdir( u32 d, const char **pname );
int os_closedir( u32 d );

#endif

