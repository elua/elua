// eLua RPC mechanism

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "eluarpc.h"
#include "rtype.h"

static u8 eluarpc_err_flag;

// *****************************************************************************
// Internal functions: fdata serialization

static u8 *eluarpc_write_u8( u8 *p, u8 fdata )
{
  *p ++ = TYPE_INT_8;
  *p ++ = fdata;
  return p;  
}

static u8* eluarpc_write_op_id( u8 *p, u8 fdata )
{
  *p ++ = TYPE_OP_ID;
  *p ++ = fdata;
  return p;    
}

static u8 *eluarpc_write_u16( u8 *p, u16 fdata )
{
  *p ++ = TYPE_INT_16;
  *p ++ = fdata & 0xFF;
  *p ++ = ( fdata >> 8 ) & 0xFF;
  return p;    
}

static u8 *eluarpc_write_u32( u8 *p, u32 fdata )
{
  *p ++ = TYPE_INT_32;
  *p ++ = fdata & 0xFF;
  *p ++ = ( fdata >> 8 ) & 0xFF;
  *p ++ = ( fdata >> 16 ) & 0xFF;
  *p ++ = ( fdata >> 24 ) & 0xFF;
  return p;        
}

static u8 *eluarpc_write_ptr( u8 *p, const void* src, u32 srclen )
{
  *p ++ = TYPE_PTR;
  p = eluarpc_write_u32( p, srclen );
  if( src )
    memcpy( p, src, srclen );
  return p + srclen;    
}

static u8 *eluarpc_write_small_ptr( u8 *p, const void* src, u16 srclen )
{
  *p ++ = TYPE_SMALL_PTR;
  p = eluarpc_write_u16( p, srclen );
  if( src )
    memcpy( p, src, srclen );
  return p + srclen;    
}

// *****************************************************************************
// Internal functions: fdata deserialization

static const u8* eluarpc_read_expect( const u8 *p, u8 fdata )
{
  if( *p ++ != fdata )
    eluarpc_err_flag = ELUARPC_ERR;
  return p;
}

static const u8 *eluarpc_read_u8( const u8 *p, u8 *pfdata )
{
  p = eluarpc_read_expect( p, TYPE_INT_8 );
  *pfdata = *p ++;
  return p;  
}

static const u8 *eluarpc_read_op_id( const u8 *p, u8 *pfdata )
{
  p = eluarpc_read_expect( p, TYPE_OP_ID );
  *pfdata = *p ++;
  return p;  
}

static const u8* eluarpc_expect_op_id( const u8 *p, u8 id )
{
  u8 temp;
  
  p = eluarpc_read_expect( p, TYPE_OP_ID );
  temp = *p ++;
  if( temp != id )
    eluarpc_err_flag = ELUARPC_ERR;
  return p;
}

static const u8 *eluarpc_read_u16( const u8 *p, u16 *pfdata )
{
  p = eluarpc_read_expect( p, TYPE_INT_16 );
  *pfdata = *p ++;
  *pfdata |= ( u32 )( *p ++ ) << 8;  
  return p;    
}

static const u8 *eluarpc_read_u32( const u8 *p, u32 *pfdata )
{
  p = eluarpc_read_expect( p, TYPE_INT_32 );
  *pfdata = *p ++;                         
  *pfdata |= ( u32 )( *p ++ ) << 8;
  *pfdata |= ( u32 )( *p ++ ) << 16;
  *pfdata |= ( u32 )( *p ++ ) << 24;      
  return p;        
}

static const u8 *eluarpc_read_ptr( const u8 *p, void* src, u32 *psrclen )
{                                         
  p = eluarpc_read_expect( p, TYPE_PTR );
  p = eluarpc_read_u32( p, psrclen );
  if( src && p )
    memcpy( src, p, *psrclen );
  return p + *psrclen;    
}

static const u8 *eluarpc_read_small_ptr( const u8 *p, void* src, u16 *psrclen )
{                                         
  p = eluarpc_read_expect( p, TYPE_SMALL_PTR );
  p = eluarpc_read_u16( p, psrclen );
  if( src && p )
    memcpy( src, p, *psrclen );
  return p + *psrclen;    
}


// *****************************************************************************
// Internal functions: packet handling (read and write)

static u8* eluarpc_packet_ptr;

static u8* eluarpc_start_packet( u8 *p )
{
  eluarpc_packet_ptr = p;
  p += ELUARPC_START_OFFSET;
  *p ++ = TYPE_START;
  p = eluarpc_write_u32( p, PACKET_SIG );
  return p;
}

static u8* eluarpc_end_packet( u8 *p )
{
  u16 len;
  
  *p ++ = TYPE_END;
  p = eluarpc_write_u32( p, ~PACKET_SIG );
  len = p - eluarpc_packet_ptr;
  p = eluarpc_packet_ptr;
  *p ++ = TYPE_PKT_SIZE;
  eluarpc_write_u16( p, len );  
  return p;  
}

static const u8* eluarpc_match_packet_start( const u8 *p )
{
  u32 fdata;
  
  p += ELUARPC_START_OFFSET;
  p = eluarpc_read_expect( p, TYPE_START );
  p = eluarpc_read_u32( p, &fdata );
  if( fdata != PACKET_SIG )
    eluarpc_err_flag = ELUARPC_ERR;
  return p;
}

static const u8* eluarpc_match_packet_end( const u8 *p )
{
  u32 fdata;
  
  p = eluarpc_read_expect( p, TYPE_END );
  p = eluarpc_read_u32( p, &fdata );
  if( fdata != ~PACKET_SIG )
    eluarpc_err_flag = ELUARPC_ERR;
  return p;
}

// *****************************************************************************
// Function serialization and deserialization

int eluarpc_get_request_id( const u8 *p, u8 *pid )
{ 
  eluarpc_err_flag = ELUARPC_OK;
  p = eluarpc_match_packet_start( p );
  p = eluarpc_read_op_id( p, pid );
  return eluarpc_err_flag;
}

u32 eluarpc_replace_flag( u32 val, u32 origflag, u32 newflag )
{
  return ( val & origflag ) ? newflag : 0; 
}

int eluarpc_get_packet_size( const u8 *p, u16 *psize )
{
  eluarpc_err_flag = ELUARPC_OK;
  p = eluarpc_read_expect( p, TYPE_PKT_SIZE );
  p = eluarpc_read_u16( p, psize );
  return eluarpc_err_flag;
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
//             P - ptr (given as ptr, len, len is an u16)
void eluarpc_gen_write( u8 *p, const char *fmt, ... )
{
  va_list ap;
  const void *ptr;
  u32 ptrlen;
  
  va_start( ap, fmt );
  p = eluarpc_start_packet( p );
  while( *fmt )
    switch( *fmt ++ )
    {
      case 'o':
        p = eluarpc_write_op_id( p, va_arg( ap, int ) );
        break;
        
      case 'r':
        *p++ = ELUARPC_OP_RES_MOD | ( u8 )va_arg( ap, int );
        break;
        
      case 'c':
        p = eluarpc_write_u8( p, ( u8 )va_arg( ap, int ) );
        break;
        
      case 'h':
        p = eluarpc_write_u16( p, ( u16 )va_arg( ap, int ) );
        break;

      case 'i':
        p = eluarpc_write_u32( p, ( u32 )va_arg( ap, int ) );
        break;
        
      case 'l':
        p = eluarpc_write_u32( p, ( u32 )va_arg( ap, u32 ) );
        break;

      case 'L':
        p = eluarpc_write_u32( p, ( u32 )va_arg( ap, s32 ) );
        break;         
      
      case 'p':
        ptr = va_arg( ap, void* );
        ptrlen = ( u32 )va_arg( ap, u32 );
        p = eluarpc_write_ptr( p, ptr, ptrlen );
        break;
        
      case 'P':
        ptr = va_arg( ap, void * );
        ptrlen = ( u16 )va_arg( ap, int );
        p = eluarpc_write_small_ptr( p, ptr, ptrlen );
        break;        
    }
  eluarpc_end_packet( p );
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
//             P - ptr (returned as ptr, len, len is an u16)
int eluarpc_gen_read( const u8 *p, const char *fmt, ... )
{
  va_list ap;
  const void *pptr;
  u32 *ptrlen;
  const u8 *tempptr;
  u32 temp32;
  u16 temp16;
  u16 *sptrlen;
  
  va_start( ap, fmt );
  eluarpc_err_flag = ELUARPC_OK;
  p = eluarpc_match_packet_start( p );
  while( *fmt )
    switch( *fmt ++ )
    {
      case 'o':
        p = eluarpc_expect_op_id( p, va_arg( ap, int ) );
        break;
        
      case 'r':
        p = eluarpc_read_expect( p, ELUARPC_OP_RES_MOD | ( u8 )va_arg( ap, int ) );
        break;
        
      case 'c':
        p = eluarpc_read_u8( p, ( u8* )va_arg( ap, void * ) );
        break;
        
      case 'h':
        p = eluarpc_read_u16( p, ( u16* )va_arg( ap, void * ) );
        break;
        
      case 'l':
        p = eluarpc_read_u32( p, ( u32* )va_arg( ap, void * ) );
        break;     

      case 'L':
        p = eluarpc_read_u32( p, &temp32 );        
        *( s32 *)va_arg( ap, void * ) = ( s32 )temp32;
        break;     
        
      case 'i':
        p = eluarpc_read_u32( p, &temp32 );
        *( int* )va_arg( ap, void * ) = ( int )temp32;        
        break;     
      
      case 'p':
        pptr = va_arg( ap, void** );
        ptrlen = ( u32* )va_arg( ap, void* );
        tempptr = p;
        p = eluarpc_read_ptr( p, NULL, &temp32 );
        if( p == tempptr + ELUARPC_PTR_HEADER_SIZE )
          *( const u8** )pptr = NULL;
        else
          *( const u8** )pptr = tempptr + ELUARPC_PTR_HEADER_SIZE;
        if( ptrlen )
          *ptrlen = temp32;        
        break;
        
      case 'P':
        pptr = va_arg( ap, void** );
        sptrlen = ( u16* )va_arg( ap, void* );
        tempptr = p;
        p = eluarpc_read_small_ptr( p, NULL, &temp16 );
        if( p == tempptr + ELUARPC_SMALL_PTR_HEADER_SIZE )
          *( const u8** )pptr = NULL;
        else
          *( const u8** )pptr = tempptr + ELUARPC_SMALL_PTR_HEADER_SIZE;
        if( sptrlen )
          *sptrlen = temp16;        
        break;        
    }
  eluarpc_match_packet_end( p );  
  return eluarpc_err_flag;
}
