// Remote file system implementation

#include <string.h>
#include <stdarg.h>
#include "type.h"
#include "remotefs.h"
#include "eluarpc.h"
#include "rtype.h"

// *****************************************************************************
// Operation: open
// open: int open( const char *pathname,int flags, mode_t mode )

void remotefs_open_write_response( u8 *p, int result )
{
  eluarpc_gen_write( p, "ri", RFS_OP_OPEN, result );
}

int remotefs_open_read_response( const u8 *p, int *presult )
{
  return eluarpc_gen_read( p, "ri", RFS_OP_OPEN, presult );  
}

void remotefs_open_write_request( u8 *p, const char* pathname, int flags, int mode )
{
  eluarpc_gen_write( p, "opii", RFS_OP_OPEN, pathname, strlen( pathname ) + 1, flags, mode );
}

int remotefs_open_read_request( const u8 *p, const char **ppathname, int *pflags, int *pmode )
{  
  return eluarpc_gen_read( p, "opii", RFS_OP_OPEN, ppathname, NULL, pflags, pmode );  
}  

// *****************************************************************************
// Operation: write
// write: ssize_t write( int fd, const void *buf, size_t count )
 
void remotefs_write_write_response( u8 *p, u32 result )
{                                                   
  eluarpc_gen_write( p, "rl", RFS_OP_WRITE, result );  
}

int remotefs_write_read_response( const u8 *p, u32 *presult )
{
  return eluarpc_gen_read( p, "rl", RFS_OP_WRITE, presult );  
}

void remotefs_write_write_request( u8 *p, int fd, const void *buf, u32 count )
{
  eluarpc_gen_write( p, "oip", RFS_OP_WRITE, fd, buf, count ); 
}

int remotefs_write_read_request( const u8 *p, int *pfd, const void **pbuf, u32 *pcount )
{
  return eluarpc_gen_read( p, "oip", RFS_OP_WRITE, pfd, pbuf, pcount );  
}

// *****************************************************************************
// Operation: read
// read: ssize_t read( int fd, void *buf, size_t count )

void remotefs_read_write_response( u8 *p, u32 readbytes )
{
  eluarpc_gen_write( p, "rp", RFS_OP_READ, NULL, readbytes );  
}

int remotefs_read_read_response( const u8 *p, const u8 **ppdata, u32 *preadbytes )
{
  return eluarpc_gen_read( p, "rp", RFS_OP_READ, ppdata, preadbytes );  
}

void remotefs_read_write_request( u8 *p, int fd, u32 count )
{
  eluarpc_gen_write( p, "oil", RFS_OP_READ, fd, count ); 
}

int remotefs_read_read_request( const u8 *p, int *pfd, u32 *pcount )
{
  return eluarpc_gen_read( p, "oil", RFS_OP_READ, pfd, pcount );    
}
  
// *****************************************************************************
// Operation: close  
// close: int close( int fd )
  
void remotefs_close_write_response( u8 *p, int result )
{
  eluarpc_gen_write( p, "ri", RFS_OP_CLOSE, result );  
}

int remotefs_close_read_response( const u8 *p, int *presult )
{
  return eluarpc_gen_read( p, "ri", RFS_OP_CLOSE, presult );   
}

void remotefs_close_write_request( u8 *p, int fd )
{
  eluarpc_gen_write( p, "oi", RFS_OP_CLOSE, fd ); 
}

int remotefs_close_read_request( const u8 *p, int *pfd )
{
  return eluarpc_gen_read( p, "oi", RFS_OP_CLOSE, pfd );  
}

// *****************************************************************************
// Operation: lseek
// lseek: lseek( int fd, off_t offset, int whence )

void remotefs_lseek_write_response( u8 *p, s32 result )
{
  eluarpc_gen_write( p, "rL", RFS_OP_LSEEK, result );
}

int remotefs_lseek_read_response( const u8 *p, s32 *presult )
{
  return eluarpc_gen_read( p, "rL", RFS_OP_LSEEK, presult );   
}

void remotefs_lseek_write_request( u8 *p, int fd, s32 offset, int whence )
{
  eluarpc_gen_write( p, "oiLi", RFS_OP_LSEEK, fd, offset, whence ); 
}

int remotefs_lseek_read_request( const u8 *p, int *pfd, s32 *poffset, int *pwhence )
{
  return eluarpc_gen_read( p, "oiLi", RFS_OP_LSEEK, pfd, poffset, pwhence );
}

// ****************************************************************************
// Operation: opendir
// opendir: u32 opendir( const char* name )

void remotefs_opendir_write_response( u8 *p, u32 d )
{
  eluarpc_gen_write( p, "rl", RFS_OP_OPENDIR, d );
}

int remotefs_opendir_read_response( const u8 *p, u32 *pd )
{
  return eluarpc_gen_read( p, "rl", RFS_OP_OPENDIR, pd );
}

void remotefs_opendir_write_request( u8 *p, const char* name )
{
  eluarpc_gen_write( p, "op", RFS_OP_OPENDIR, name, strlen( name ) + 1 );
}

int remotefs_opendir_read_request( const u8 *p, const char **pname )
{
  return eluarpc_gen_read( p, "op", RFS_OP_OPENDIR, pname, NULL );
}

// ****************************************************************************
// Operation: readdir
// readdir: readdir( u32 d, const char **pname, u32 *psize, u32 *pftime );
// Will return fname, size, ftime as side effects in response

void remotefs_readdir_write_response( u8 *p, const char *name, u32 size, u32 ftime )
{
  eluarpc_gen_write( p, "rpll", RFS_OP_READDIR, name, name ? strlen( name ) + 1 : 0, size, ftime );
}

int remotefs_readdir_read_response( const u8 *p, const char **pname, u32 *psize, u32 *pftime )
{
  return eluarpc_gen_read( p, "rpll",  RFS_OP_READDIR, pname, NULL, psize, pftime );
}

void remotefs_readdir_write_request( u8 *p, u32 d )
{
  eluarpc_gen_write( p, "ol", RFS_OP_READDIR, d );
}

int remotefs_readdir_read_request( const u8 *p, u32 *pd )
{
  return eluarpc_gen_read( p, "ol", RFS_OP_READDIR, pd );
}

// ****************************************************************************
// Operation: closedir
// closedir: int closedir( u32 d )

void remotefs_closedir_write_response( u8 *p, int d )
{
  eluarpc_gen_write( p, "ri", RFS_OP_CLOSEDIR, d );
}

int remotefs_closedir_read_response( const u8 *p, int *pd )
{
  return eluarpc_gen_read( p, "ri", RFS_OP_CLOSEDIR, pd );
}

void remotefs_closedir_write_request( u8 *p, u32 d )
{
  eluarpc_gen_write( p, "ol", RFS_OP_CLOSEDIR, d );
}

int remotefs_closedir_read_request( const u8 *p, u32 *pd )
{
  return eluarpc_gen_read( p, "ol", RFS_OP_CLOSEDIR, pd );
}


