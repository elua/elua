// Remote file system implementation

#ifndef __REMOTEFS_H__
#define __REMOTEFS_H__

#include "type.h"

// Operation IDs
#define   RFS_OP_OPEN     0x01
#define   RFS_OP_FIRST    RFS_OP_OPEN
#define   RFS_OP_WRITE    0x02
#define   RFS_OP_READ     0x03
#define   RFS_OP_CLOSE    0x04
#define   RFS_OP_LSEEK    0x05
#define   RFS_OP_OPENDIR  0x06
#define   RFS_OP_READDIR  0x07
#define   RFS_OP_CLOSEDIR 0x08
#define   RFS_OP_LAST     RFS_OP_CLOSEDIR
#define   RFS_OP_RES_MOD  0x80

// Platform independent constants for "flags" in "open"
#define   RFS_OPEN_FLAG_APPEND      0x01
#define   RFS_OPEN_FLAG_CREAT       0x02
#define   RFS_OPEN_FLAG_EXCL        0x04
#define   RFS_OPEN_FLAG_TRUNC       0x08
#define   RFS_OPEN_FLAG_SYNC        0x10
#define   RFS_OPEN_FLAG_RDONLY      0x20
#define   RFS_OPEN_FLAG_WRONLY      0x40
#define   RFS_OPEN_FLAG_RDWR        0x80

// Platform independent seek modes for "seek"
#define   RFS_LSEEK_SET             0x01
#define   RFS_LSEEK_CUR             0x02
#define   RFS_LSEEK_END             0x03

// R/W pipe names (used only with the simulator)
#define   RFS_SRV_WRITE_PIPE        "/tmp/elua_srv_write"
#define   RFS_SRV_READ_PIPE         "/tmp/elua_srv_read"

// Max filename size on a RFS instance
#define   RFS_MAX_FNAME_SIZE        31

// Function: int open(const char *pathname,int flags, mode_t mode)
void remotefs_open_write_response( u8 *p, int result );
int remotefs_open_read_response( const u8 *p, int *presult );
void remotefs_open_write_request( u8 *p, const char* pathname, int flags, int mode );
int remotefs_open_read_request( const u8 *p, const char **ppathname, int *pflags, int *pmode );

// Function: ssize_t write(int fd, const void *buf, size_t count)
void remotefs_write_write_response( u8 *p, u32 result );
int remotefs_write_read_response( const u8 *p, u32 *presult );
void remotefs_write_write_request( u8 *p, int fd, const void *buf, u32 count );
int remotefs_write_read_request( const u8 *p, int *pfd, const void **pbuf, u32 *pcount );

// Function: ssize_t read(int fd, void *buf, size_t count)
void remotefs_read_write_response( u8 *p, u32 readbytes );
int remotefs_read_read_response( const u8 *p, const u8 **ppdata, u32 *preadbytes );
void remotefs_read_write_request( u8 *p, int fd, u32 count );
int remotefs_read_read_request( const u8 *p, int *pfd, u32 *pcount );
                                 
// Function: int close( int fd )
void remotefs_close_write_response( u8 *p, int result );
int remotefs_close_read_response( const u8 *p, int *presult );
void remotefs_close_write_request( u8 *p, int fd );
int remotefs_close_read_request( const u8 *p, int *pfd );

// Function: off_t lseek( int fd, off_t offset, int whence )
void remotefs_lseek_write_response( u8 *p, s32 result );
int remotefs_lseek_read_response( const u8 *p, s32 *presult );
void remotefs_lseek_write_request( u8 *p, int fd, s32 offset, int whence );
int remotefs_lseek_read_request( const u8 *p, int *pfd, s32 *poffset, int *pwhence );

// Function: u32 opendir( const char* name )
void remotefs_opendir_write_response( u8 *p, u32 d );
int remotefs_opendir_read_response( const u8 *p, u32 *pd );
void remotefs_opendir_write_request( u8 *p, const char* name );
int remotefs_opendir_read_request( const u8 *p, const char **pname );

// Function: void readdir( u32 d, const char **pname, u32 *psize, u32 *pftime );
// Will return fname, size, ftime as side effects in response
void remotefs_readdir_write_response( u8 *p, const char *name, u32 size, u32 ftime );
int remotefs_readdir_read_response( const u8 *p, const char **pname, u32 *psize, u32 *pftime );
void remotefs_readdir_write_request( u8 *p, u32 d );
int remotefs_readdir_read_request( const u8 *p, u32 *pd );

// Function: int closedir( u32 d )
void remotefs_closedir_write_response( u8 *p, int d );
int remotefs_closedir_read_response( const u8 *p, int *pd );
void remotefs_closedir_write_request( u8 *p, u32 d );
int remotefs_closedir_read_request( const u8 *p, u32 *pd );

#endif

