// Remote filesystem client

#ifndef __CLIENT_H__
#define __CLIENT_H__

#include "type.h"

// Error codes
#define CLIENT_OK   0
#define CLIENT_ERR  1

// RFS client send/receive functions
typedef u32 ( *p_rfsc_send )( const u8 *p, u32 size );
typedef u32 ( *p_rfsc_recv )( u8 *p, u32 size, u32 timeout );

// Public interface
void rfsc_setup( u8 *pbuf, p_rfsc_send rfsc_send_func, p_rfsc_recv rfsc_recv_func, u32 timeout );
void rfsc_set_timeout( u32 timeout );
int rfsc_open( const char* pathname, int flags, int mode );
s32 rfsc_write( int fd, const void *buf, u32 count );
s32 rfsc_read( int fd, void *buf, u32 count );
s32 rfsc_lseek( int fd, s32 offset, int whence );
int rfsc_close( int fd );

#endif

