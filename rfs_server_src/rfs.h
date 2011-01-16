// RFS public interface (used by mux)

#ifndef __RFS_H__
#define __RFS_H__

int rfs_init( int argc, const char **argv );
void rfs_mem_start_request();
int rfs_mem_read_request_packet( int c );
int rfs_mem_has_response();
void rfs_mem_write_response( u16 *plen, u8 **pdata );

#endif
