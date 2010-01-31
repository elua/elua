// Remote file system implementation

#include <string.h>
#include <stdarg.h>
#include "type.h"
#include "remotefs.h"
#include "rtype.h"

static u8 remotefs_err_flag;

// *****************************************************************************
// Internal functions: data serialization

static u8 *remotefs_write_u8( u8 *p, u8 data )
{
  *p ++ = TYPE_INT_8;
  *p ++ = data;
  return p;  
}

static u8* remotefs_write_op_id( u8 *p, u8 data )
{
  *p ++ = TYPE_OP_ID;
  *p ++ = data;
  return p;    
}

static u8 *remotefs_write_u16( u8 *p, u16 data )
{
  *p ++ = TYPE_INT_16;
  *p ++ = data & 0xFF;
  *p ++ = ( data >> 8 ) & 0xFF;
  return p;    
}

static u8 *remotefs_write_u32( u8 *p, u32 data )
{
  *p ++ = TYPE_INT_32;
  *p ++ = data & 0xFF;
  *p ++ = ( data >> 8 ) & 0xFF;
  *p ++ = ( data >> 16 ) & 0xFF;
  *p ++ = ( data >> 24 ) & 0xFF;
  return p;        
}

static u8 *remotefs_write_ptr( u8 *p, const void* src, u32 srclen )
{
  *p ++ = TYPE_PTR;
  p = remotefs_write_u32( p, srclen );
  if( src )
    memcpy( p, src, srclen );
  return p + srclen;    
}

// *****************************************************************************
// Internal functions: data deserialization

static const u8* remotefs_read_expect( const u8 *p, u8 data )
{
  if( *p ++ != data )
    remotefs_err_flag = REMOTEFS_ERR;
  return p;
}

static const u8 *remotefs_read_u8( const u8 *p, u8 *pdata )
{
  p = remotefs_read_expect( p, TYPE_INT_8 );
  *pdata = *p ++;
  return p;  
}

static const u8 *remotefs_read_op_id( const u8 *p, u8 *pdata )
{
  p = remotefs_read_expect( p, TYPE_OP_ID );
  *pdata = *p ++;
  return p;  
}

static const u8* remotefs_expect_op_id( const u8 *p, u8 id )
{
  u8 temp;
  
  p = remotefs_read_expect( p, TYPE_OP_ID );
  temp = *p ++;
  if( temp != id )
    remotefs_err_flag = REMOTEFS_ERR;
  return p;
}

static const u8 *remotefs_read_u16( const u8 *p, u16 *pdata )
{
  p = remotefs_read_expect( p, TYPE_INT_16 );
  *pdata = *p ++;
  *pdata |= ( *p ++ ) << 8;  
  return p;    
}

static const u8 *remotefs_read_u32( const u8 *p, u32 *pdata )
{
  p = remotefs_read_expect( p, TYPE_INT_32 );
  *pdata = *p ++;                         
  *pdata |= ( *p ++ ) << 8;
  *pdata |= ( *p ++ ) << 16;
  *pdata |= ( *p ++ ) << 24;      
  return p;        
}

static const u8 *remotefs_read_ptr( const u8 *p, void* src, u32 *psrclen )
{                                         
  p = remotefs_read_expect( p, TYPE_PTR );
  p = remotefs_read_u32( p, psrclen );
  if( src && p )
    memcpy( src, p, *psrclen );
  return p + *psrclen;    
}

// *****************************************************************************
// Internal functions: packet handling (read and write)

static u8* remotefs_packet_ptr;

static u8* remotefs_start_packet( u8 *p )
{
  remotefs_packet_ptr = p;
  p += RFS_START_OFFSET;
  *p ++ = TYPE_START;
  p = remotefs_write_u32( p, PACKET_SIG );
  return p;
}

static u8* remotefs_end_packet( u8 *p )
{
  u16 len;
  
  *p ++ = TYPE_END;
  p = remotefs_write_u32( p, ~PACKET_SIG );
  len = p - remotefs_packet_ptr;
  p = remotefs_packet_ptr;
  *p ++ = TYPE_PKT_SIZE;
  remotefs_write_u16( p, len );  
  return p;  
}

static const u8* remotefs_match_packet_start( const u8 *p )
{
  u32 data;
  
  p += RFS_START_OFFSET;
  p = remotefs_read_expect( p, TYPE_START );
  p = remotefs_read_u32( p, &data );
  if( data != PACKET_SIG )
    remotefs_err_flag = REMOTEFS_ERR;
  return p;
}

static const u8* remotefs_match_packet_end( const u8 *p )
{
  u32 data;
  
  p = remotefs_read_expect( p, TYPE_END );
  p = remotefs_read_u32( p, &data );
  if( data != ~PACKET_SIG )
    remotefs_err_flag = REMOTEFS_ERR;
  return p;
}

// *****************************************************************************
// Function serialization and deserialization

int remotefs_get_request_id( const u8 *p, u8 *pid )
{ 
  remotefs_err_flag = REMOTEFS_OK;
  p = remotefs_match_packet_start( p );
  p = remotefs_read_op_id( p, pid );
  return remotefs_err_flag;
}

u32 remotefs_replace_flag( u32 val, u32 origflag, u32 newflag )
{
  return ( val & origflag ) ? newflag : 0; 
}

int remotefs_get_packet_size( const u8 *p, u16 *psize )
{
  remotefs_err_flag = REMOTEFS_OK;
  p = remotefs_read_expect( p, TYPE_PKT_SIZE );
  p = remotefs_read_u16( p, psize );
  return remotefs_err_flag;
}

// Generic write function
// Specifiers: o - operation
//             r - response
//             c - u8
//             h - u16
//             l - u32
//             i - int
//             L - s32
//             p - ptr (given as ptr, len, len is an u32)
void remotefs_gen_write( u8 *p, const char *fmt, ... )
{
  va_list ap;
  const void *ptr;
  u32 ptrlen;
  
  va_start( ap, fmt );
  p = remotefs_start_packet( p );
  while( *fmt )
    switch( *fmt ++ )
    {
      case 'o':
        p = remotefs_write_op_id( p, va_arg( ap, int ) );
        break;
        
      case 'r':
        *p++ = RFS_OP_RES_MOD | ( u8 )va_arg( ap, int );
        break;
        
      case 'c':
        p = remotefs_write_u8( p, ( u8 )va_arg( ap, int ) );
        break;
        
      case 'h':
        p = remotefs_write_u16( p, ( u16 )va_arg( ap, int ) );
        break;

      case 'i':
        p = remotefs_write_u32( p, ( u32 )va_arg( ap, int ) );
        break;
        
      case 'l':
        p = remotefs_write_u32( p, ( u32 )va_arg( ap, u32 ) );
        break;

      case 'L':
        p = remotefs_write_u32( p, ( u32 )va_arg( ap, s32 ) );
        break;         
      
      case 'p':
        ptr = va_arg( ap, void * );
        ptrlen = ( u32 )va_arg( ap, long );
        p = remotefs_write_ptr( p, ptr, ptrlen );
        break;
    }
  remotefs_end_packet( p );
}

// Generic read function
// Specifiers: o - operation
//             r - response
//             c - u8
//             h - u16
//             l - u32
//             L - s32
//             i - int
//             p - ptr (returned as ptr, len, len is an u32)
int remotefs_gen_read( const u8 *p, const char *fmt, ... )
{
  va_list ap;
  const void *pptr;
  u32 *ptrlen;
  const u8 *tempptr;
  u32 temp32;
  
  va_start( ap, fmt );
  remotefs_err_flag = REMOTEFS_OK;
  p = remotefs_match_packet_start( p );
  while( *fmt )
    switch( *fmt ++ )
    {
      case 'o':
        p = remotefs_expect_op_id( p, va_arg( ap, int ) );
        break;
        
      case 'r':
        p = remotefs_read_expect( p, RFS_OP_RES_MOD | ( u8 )va_arg( ap, int ) );
        break;
        
      case 'c':
        p = remotefs_read_u8( p, ( u8* )va_arg( ap, void* ) );
        break;
        
      case 'h':
        p = remotefs_read_u16( p, ( u16* )va_arg( ap, void* ) );
        break;
        
      case 'l':
        p = remotefs_read_u32( p, ( u32* )va_arg( ap, void* ) );
        break;     

      case 'L':
        p = remotefs_read_u32( p, &temp32 );        
        *( s32 *)va_arg( ap, void* ) = ( s32 )temp32;
        break;     
        
      case 'i':
        p = remotefs_read_u32( p, &temp32 );
        *( int* )va_arg( ap, void* ) = ( int )temp32;        
        break;     
      
      case 'p':
        pptr = va_arg( ap, void** );
        ptrlen = va_arg( ap, void* );
        tempptr = p;
        p = remotefs_read_ptr( p, NULL, &temp32 );
        if( p == tempptr + RFS_PTR_HEADER_SIZE )
          *( const u8** )pptr = NULL;
        else
          *( const u8** )pptr = tempptr + RFS_PTR_HEADER_SIZE;
        if( ptrlen )
          *ptrlen = temp32;        
        break;
    }
  remotefs_match_packet_end( p );  
  return remotefs_err_flag;
}


// *****************************************************************************
// Operation: open
// open: int open( const char *pathname,int flags, mode_t mode )

void remotefs_open_write_response( u8 *p, int result )
{
  remotefs_gen_write( p, "ri", RFS_OP_OPEN, result );
}

int remotefs_open_read_response( const u8 *p, int *presult )
{
  return remotefs_gen_read( p, "ri", RFS_OP_OPEN, presult );  
}

void remotefs_open_write_request( u8 *p, const char* pathname, int flags, int mode )
{
  remotefs_gen_write( p, "opii", RFS_OP_OPEN, pathname, strlen( pathname ) + 1, flags, mode );
}

int remotefs_open_read_request( const u8 *p, const char **ppathname, int *pflags, int *pmode )
{  
  return remotefs_gen_read( p, "opii", RFS_OP_OPEN, ppathname, NULL, pflags, pmode );  
}  

// *****************************************************************************
// Operation: write
// write: ssize_t write( int fd, const void *buf, size_t count )
 
void remotefs_write_write_response( u8 *p, u32 result )
{                                                   
  remotefs_gen_write( p, "rl", RFS_OP_WRITE, result );  
}

int remotefs_write_read_response( const u8 *p, u32 *presult )
{
  return remotefs_gen_read( p, "rl", RFS_OP_WRITE, presult );  
}

void remotefs_write_write_request( u8 *p, int fd, const void *buf, u32 count )
{
  remotefs_gen_write( p, "oip", RFS_OP_WRITE, fd, buf, count ); 
}

int remotefs_write_read_request( const u8 *p, int *pfd, const void **pbuf, u32 *pcount )
{
  return remotefs_gen_read( p, "oip", RFS_OP_WRITE, pfd, pbuf, pcount );  
}

// *****************************************************************************
// Operation: read
// read: ssize_t read( int fd, void *buf, size_t count )

void remotefs_read_write_response( u8 *p, u32 readbytes )
{
  remotefs_gen_write( p, "rp", RFS_OP_READ, NULL, readbytes );  
}

int remotefs_read_read_response( const u8 *p, const u8 **ppdata, u32 *preadbytes )
{
  return remotefs_gen_read( p, "rp", RFS_OP_READ, ppdata, preadbytes );  
}

void remotefs_read_write_request( u8 *p, int fd, u32 count )
{
  remotefs_gen_write( p, "oil", RFS_OP_READ, fd, count ); 
}

int remotefs_read_read_request( const u8 *p, int *pfd, u32 *pcount )
{
  return remotefs_gen_read( p, "oil", RFS_OP_READ, pfd, pcount );    
}
  
// *****************************************************************************
// Operation: close  
// close: int close( int fd )
  
void remotefs_close_write_response( u8 *p, int result )
{
  remotefs_gen_write( p, "ri", RFS_OP_CLOSE, result );  
}

int remotefs_close_read_response( const u8 *p, int *presult )
{
  return remotefs_gen_read( p, "ri", RFS_OP_CLOSE, presult );   
}

void remotefs_close_write_request( u8 *p, int fd )
{
  remotefs_gen_write( p, "oi", RFS_OP_CLOSE, fd ); 
}

int remotefs_close_read_request( const u8 *p, int *pfd )
{
  return remotefs_gen_read( p, "oi", RFS_OP_CLOSE, pfd );  
}

// *****************************************************************************
// Operation: lseek
// lseek: lseek( int fd, off_t offset, int whence )

void remotefs_lseek_write_response( u8 *p, s32 result )
{
  remotefs_gen_write( p, "rL", RFS_OP_LSEEK, result );
}

int remotefs_lseek_read_response( const u8 *p, s32 *presult )
{
  return remotefs_gen_read( p, "rL", RFS_OP_LSEEK, presult );   
}

void remotefs_lseek_write_request( u8 *p, int fd, s32 offset, int whence )
{
  remotefs_gen_write( p, "oiLi", RFS_OP_LSEEK, fd, offset, whence ); 
}

int remotefs_lseek_read_request( const u8 *p, int *pfd, s32 *poffset, int *pwhence )
{
  return remotefs_gen_read( p, "oiLi", RFS_OP_LSEEK, pfd, poffset, pwhence );
}

// ****************************************************************************
// Operation: opendir
// opendir: u32 opendir( const char* name )

void remotefs_opendir_write_response( u8 *p, u32 d )
{
  remotefs_gen_write( p, "rl", RFS_OP_OPENDIR, d );
}

int remotefs_opendir_read_response( const u8 *p, u32 *pd )
{
  return remotefs_gen_read( p, "rl", RFS_OP_OPENDIR, pd );
}

void remotefs_opendir_write_request( u8 *p, const char* name )
{
  remotefs_gen_write( p, "op", RFS_OP_OPENDIR, name, strlen( name ) + 1 );
}

int remotefs_opendir_read_request( const u8 *p, const char **pname )
{
  return remotefs_gen_read( p, "op", RFS_OP_OPENDIR, pname, NULL );
}

// ****************************************************************************
// Operation: readdir
// readdir: readdir( u32 d, const char **pname, u32 *psize, u32 *pftime );
// Will return fname, size, ftime as side effects in response

void remotefs_readdir_write_response( u8 *p, const char *name, u32 size, u32 ftime )
{
  remotefs_gen_write( p, "rpll", RFS_OP_READDIR, name, name ? strlen( name ) + 1 : 0, size, ftime );
}

int remotefs_readdir_read_response( const u8 *p, const char **pname, u32 *psize, u32 *pftime )
{
  return remotefs_gen_read( p, "rpll",  RFS_OP_READDIR, pname, NULL, psize, pftime );
}

void remotefs_readdir_write_request( u8 *p, u32 d )
{
  remotefs_gen_write( p, "ol", RFS_OP_READDIR, d );
}

int remotefs_readdir_read_request( const u8 *p, u32 *pd )
{
  return remotefs_gen_read( p, "ol", RFS_OP_READDIR, pd );
}

// ****************************************************************************
// Operation: closedir
// closedir: int closedir( u32 d )

void remotefs_closedir_write_response( u8 *p, int d )
{
  remotefs_gen_write( p, "ri", RFS_OP_CLOSEDIR, d );
}

int remotefs_closedir_read_response( const u8 *p, int *pd )
{
  return remotefs_gen_read( p, "ri", RFS_OP_CLOSEDIR, pd );
}

void remotefs_closedir_write_request( u8 *p, u32 d )
{
  remotefs_gen_write( p, "rl", RFS_OP_CLOSEDIR, d );
}

int remotefs_closedir_read_request( const u8 *p, u32 *pd )
{
  return remotefs_gen_read( p, "rl", RFS_OP_CLOSEDIR, pd );
}


