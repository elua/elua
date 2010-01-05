// Remote file system implementation

#ifndef __REMOTEFS_H__
#define __REMOTEFS_H__

#include "type.h"

#define   PACKET_SIG      0x18AFC284UL

// Error codes
#define   REMOTEFS_OK     0
#define   REMOTEFS_ERR    1

// Operation IDs
#define   RFS_OP_OPEN     0x01
#define   RFS_OP_FIRST    RFS_OP_OPEN
#define   RFS_OP_WRITE    0x02
#define   RFS_OP_READ     0x03
#define   RFS_OP_CLOSE    0x04
#define   RFS_OP_LSEEK    0x05
#define   RFS_OP_LAST     RFS_OP_LSEEK
#define   RFS_OP_RES_MOD  0x80

// Protocol constants
#define   RFS_START_OFFSET    4
#define   RFS_START_SIZE      6
#define   RFS_END_SIZE        6
#define   RFS_RESPONSE_SIZE   1
#define   RFS_PTR_HEADER_SIZE 6
#define   RFS_U32_SIZE        5
#define   RFS_OP_ID_SIZE      2
#define   RFS_READ_BUF_OFFSET ( RFS_START_OFFSET + RFS_START_SIZE + RFS_RESPONSE_SIZE + RFS_PTR_HEADER_SIZE )
#define   RFS_WRITE_REQUEST_EXTRA ( RFS_START_OFFSET + RFS_START_SIZE + RFS_OP_ID_SIZE + RFS_U32_SIZE + RFS_PTR_HEADER_SIZE + RFS_END_SIZE )

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

// Public interface

// Get request ID
int remotefs_get_request_id( const u8 *p, u8 *pid );

// Replace a flag with another flag
u32 remotefs_replace_flag( u32 val, u32 origflag, u32 newflag );

// Get packet size
int remotefs_get_packet_size( const u8 *p, u16 *psize );

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

#endif
