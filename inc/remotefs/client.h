// Remote filesystem client

#ifndef __CLIENT_H__
#define __CLIENT_H__

#include "type.h"
#include "platform.h"

// Error codes
#define CLIENT_OK   0
#define CLIENT_ERR  1

// RFS client send/receive functions
typedef u32 ( *p_rfsc_send )( const u8 *p, u32 size );
typedef u32 ( *p_rfsc_recv )( u8 *p, u32 size, timer_data_type timeout );

// Public interface
void rfsc_setup( u8 *pbuf, p_rfsc_send rfsc_send_func, p_rfsc_recv rfsc_recv_func, timer_data_type timeout );
void rfsc_set_timeout( timer_data_type timeout );
int rfsc_open( const char* pathname, int flags, int mode );
s32 rfsc_write( int fd, const void *buf, u32 count );
s32 rfsc_read( int fd, void *buf, u32 count );
s32 rfsc_lseek( int fd, s32 offset, int whence );
int rfsc_close( int fd );
u32 rfsc_opendir( const char* name );
void rfsc_readdir( u32 d, const char **pname, u32 *psize, u32 *ptime );
int rfsc_closedir( u32 d );

#endif

