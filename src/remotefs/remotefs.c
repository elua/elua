// Remote file system implementation

#include <string.h>
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
  if( src )
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

// open: int open(const char *pathname,int flags, mode_t mode)
void remotefs_open_write_response( u8 *p, int result )
{
  p = remotefs_start_packet( p );
  *p ++ = RFS_OP_RES_MOD | RFS_OP_OPEN;
  p = remotefs_write_u32( p, ( u32 )result );
  remotefs_end_packet( p );  
}

int remotefs_open_read_response( const u8 *p, int *presult )
{
  u32 res = 0;
  
  remotefs_err_flag = REMOTEFS_OK;
  p = remotefs_match_packet_start( p );
  p = remotefs_read_expect( p, RFS_OP_RES_MOD | RFS_OP_OPEN );
  p = remotefs_read_u32( p, &res );
  *presult = ( int )res;
  remotefs_match_packet_end( p );  
  return remotefs_err_flag;   
}

void remotefs_open_write_request( u8 *p, const char* pathname, int flags, int mode )
{
  p = remotefs_start_packet( p );
  p = remotefs_write_op_id( p, RFS_OP_OPEN );
  p = remotefs_write_ptr( p, pathname, strlen( pathname ) + 1 );
  p = remotefs_write_u32( p, ( u32 )flags );
  p = remotefs_write_u32( p, ( u32 )mode );
  remotefs_end_packet( p );  
}

int remotefs_open_read_request( const u8 *p, const char **ppathname, int *pflags, int *pmode )
{
  u32 temp32 = 0;
  const u8 *tempptr = NULL;
  
  remotefs_err_flag = REMOTEFS_OK;
  p = remotefs_match_packet_start( p );
  p = remotefs_expect_op_id( p, RFS_OP_OPEN );
  tempptr = p;
  p = remotefs_read_ptr( p, NULL, &temp32 );
  *ppathname = ( const char * )tempptr + RFS_PTR_HEADER_SIZE;
  p = remotefs_read_u32( p, &temp32 );
  *pflags = ( int )temp32;
  p = remotefs_read_u32( p, &temp32 );
  *pmode = ( int )temp32;
  remotefs_match_packet_end( p );  
  return remotefs_err_flag;         
}  

// write: ssize_t write(int fd, const void *buf, size_t count); 
void remotefs_write_write_response( u8 *p, u32 result )
{
  p = remotefs_start_packet( p );
  *p ++ = RFS_OP_RES_MOD | RFS_OP_WRITE;
  p = remotefs_write_u32( p, result );
  remotefs_end_packet( p );  
}

int remotefs_write_read_response( const u8 *p, u32 *presult )
{
  remotefs_err_flag = REMOTEFS_OK;
  p = remotefs_match_packet_start( p );
  p = remotefs_read_expect( p, RFS_OP_RES_MOD | RFS_OP_WRITE );
  p = remotefs_read_u32( p, presult );
  remotefs_match_packet_end( p );  
  return remotefs_err_flag;   
}

void remotefs_write_write_request( u8 *p, int fd, const void *buf, u32 count )
{
  p = remotefs_start_packet( p );
  p = remotefs_write_op_id( p, RFS_OP_WRITE );
  p = remotefs_write_u32( p, ( u32 )fd );
  p = remotefs_write_ptr( p, buf, count );
  remotefs_end_packet( p ); 
}

int remotefs_write_read_request( const u8 *p, int *pfd, const void **pbuf, u32 *pcount )
{
  u32 temp32 = 0;
  const u8 *tempptr;
  
  remotefs_err_flag = REMOTEFS_OK;
  p = remotefs_match_packet_start( p );
  p = remotefs_expect_op_id( p, RFS_OP_WRITE );
  p = remotefs_read_u32( p, &temp32 );
  *pfd = ( int )temp32;  
  tempptr = p;
  p = remotefs_read_ptr( p, NULL, pcount );
  *pbuf = tempptr + RFS_PTR_HEADER_SIZE;
  remotefs_match_packet_end( p );  
  return remotefs_err_flag;    
}

// Function: ssize_t read(int fd, void *buf, size_t count)
// Assumes that the read data is already in the buffer
void remotefs_read_write_response( u8 *p, u32 readbytes )
{
  p = remotefs_start_packet( p );
  *p ++ = RFS_OP_RES_MOD | RFS_OP_READ;
  p = remotefs_write_ptr( p, NULL, readbytes );
  remotefs_end_packet( p );  
}

int remotefs_read_read_response( const u8 *p, const u8 **ppdata, u32 *preadbytes )
{
  const u8 *tempptr;
    
  remotefs_err_flag = REMOTEFS_OK;
  p = remotefs_match_packet_start( p );
  p = remotefs_read_expect( p, RFS_OP_RES_MOD | RFS_OP_READ );
  tempptr = p;
  p = remotefs_read_ptr( p, NULL, preadbytes );
  *ppdata = tempptr + RFS_PTR_HEADER_SIZE;
  remotefs_match_packet_end( p );  
  return remotefs_err_flag; 
}

void remotefs_read_write_request( u8 *p, int fd, u32 count )
{
  p = remotefs_start_packet( p );
  p = remotefs_write_op_id( p, RFS_OP_READ );
  p = remotefs_write_u32( p, ( u32 )fd );
  p = remotefs_write_u32( p, count );
  remotefs_end_packet( p ); 
}

int remotefs_read_read_request( const u8 *p, int *pfd, u32 *pcount )
{
  u32 temp32;
  
  remotefs_err_flag = REMOTEFS_OK;
  p = remotefs_match_packet_start( p );
  p = remotefs_expect_op_id( p, RFS_OP_READ );
  p = remotefs_read_u32( p, &temp32 );
  *pfd = ( int )temp32;
  p = remotefs_read_u32( p, pcount );
  remotefs_match_packet_end( p );
  return remotefs_err_flag;  
}
  
// close: int close( int fd )  
void remotefs_close_write_response( u8 *p, int result )
{
  p = remotefs_start_packet( p );
  *p ++ = RFS_OP_RES_MOD | RFS_OP_CLOSE;
  p = remotefs_write_u32( p, ( u32 )result );
  remotefs_end_packet( p );  
}

int remotefs_close_read_response( const u8 *p, int *presult )
{
  u32 res = 0;
  
  remotefs_err_flag = REMOTEFS_OK;
  p = remotefs_match_packet_start( p );
  p = remotefs_read_expect( p, RFS_OP_RES_MOD | RFS_OP_CLOSE );
  p = remotefs_read_u32( p, &res );
  *presult = ( int )res;
  remotefs_match_packet_end( p );  
  return remotefs_err_flag;   
}

void remotefs_close_write_request( u8 *p, int fd )
{
  p = remotefs_start_packet( p );
  p = remotefs_write_op_id( p, RFS_OP_CLOSE );
  p = remotefs_write_u32( p, ( u32 )fd );
  remotefs_end_packet( p ); 
}

int remotefs_close_read_request( const u8 *p, int *pfd )
{
  u32 temp32;
  
  remotefs_err_flag = REMOTEFS_OK;
  p = remotefs_match_packet_start( p );
  p = remotefs_expect_op_id( p, RFS_OP_CLOSE );
  p = remotefs_read_u32( p, &temp32 );
  *pfd = ( int )temp32;
  remotefs_match_packet_end( p );
  return remotefs_err_flag;  
}

// lseek: lseek( int fd, off_t offset, int whence )
void remotefs_lseek_write_response( u8 *p, s32 result )
{
  p = remotefs_start_packet( p );
  *p ++ = RFS_OP_RES_MOD | RFS_OP_LSEEK;
  p = remotefs_write_u32( p, ( u32 )result );
  remotefs_end_packet( p );  
}

int remotefs_lseek_read_response( const u8 *p, s32 *presult )
{
  u32 temp;

  remotefs_err_flag = REMOTEFS_OK;
  p = remotefs_match_packet_start( p );
  p = remotefs_read_expect( p, RFS_OP_RES_MOD | RFS_OP_LSEEK );
  p = remotefs_read_u32( p, &temp );
  *presult = ( s32 )temp;
  remotefs_match_packet_end( p );  
  return remotefs_err_flag;   
}

void remotefs_lseek_write_request( u8 *p, int fd, s32 offset, int whence )
{
  p = remotefs_start_packet( p );
  p = remotefs_write_op_id( p, RFS_OP_LSEEK );
  p = remotefs_write_u32( p, ( u32 )fd );
  p = remotefs_write_u32( p, ( u32 )offset );
  p = remotefs_write_u8( p, ( u8 )whence );
  remotefs_end_packet( p ); 
}

int remotefs_lseek_read_request( const u8 *p, int *pfd, s32 *poffset, int *pwhence )
{
  u32 temp32;
  u8 temp8;
  
  remotefs_err_flag = REMOTEFS_OK;
  p = remotefs_match_packet_start( p );
  p = remotefs_expect_op_id( p, RFS_OP_LSEEK );
  p = remotefs_read_u32( p, &temp32 );
  *pfd = ( int )temp32;
  p = remotefs_read_u32( p, &temp32 );
  *poffset = ( s32 )temp32;
  p = remotefs_read_u8( p, &temp8 );
  *pwhence = ( int )temp8;
  remotefs_match_packet_end( p );
  return remotefs_err_flag;
}

