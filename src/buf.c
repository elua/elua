// eLua "char device" buffering system

#include "platform_conf.h"
#include <stdio.h>

#if defined( BUF_ENABLE_UART ) || defined( BUF_ENABLE_ADC )
#define BUF_ENABLE
#endif

#if defined( BUILD_USB_CDC )
#define NUM_CDC_UART    1
#else
#define NUM_CDC_UART    0
#endif

#ifdef BUILD_SERMUX
#define NUM_VUART_BUFS  SERMUX_NUM_VUART
#else
#define NUM_VUART_BUFS  0
#endif

#define NUM_TOTAL_UART  ( NUM_UART + NUM_VUART_BUFS + NUM_CDC_UART )

#ifdef BUF_ENABLE

#include "type.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>

// [TODO]? Following code might need a C99 compiler (for 0-sized arrays)
#ifdef BUF_ENABLE_UART
  static buf_desc buf_desc_uart[ NUM_TOTAL_UART ];
#else
  static buf_desc buf_desc_uart[ 0 ];
#endif

#ifdef BUF_ENABLE_ADC
  static buf_desc buf_desc_adc [ NUM_ADC ];
#else
  static buf_desc buf_desc_adc [ 0 ];
#endif

// NOTE: the order of descriptors here MUST match the order of the BUF_ID_xx
// enum in inc/buf.h
static const buf_desc* buf_desc_array[ BUF_ID_TOTAL ] = 
{
  buf_desc_uart,
  buf_desc_adc
};

// Helper macros
#define BUF_MOD_INCR( p, m ) p->m = ( p->m + ( ( u16 )1 << p->logdsize ) ) & ( ( ( u16 )1 << p->logsize ) - 1 )
#define BUF_REALSIZE( p ) ( ( u16 )1 << ( p->logsize - p->logdsize ) )
#define BUF_BYTESIZE( p ) ( ( u16 )1 << p->logsize )
#define BUF_REALDSIZE( p ) ( ( u16 )1 << p->logdsize )
#define BUF_GETPTR( resid, resnum ) buf_desc *pbuf = ( buf_desc* )buf_desc_array[ resid ] + resnum

// READ16 and WRITE16 macros are here to ensure _atomic_ reads and writes of 
// 16-bits data. Might have to be changed for an 8-bit architecture.
#define READ16( p )     p
#define WRITE16( p, x ) p = x

// Helper: check 'resnum' (for virtual UARTs)
// UART resource ID translation to buffer ID translation (for serial multiplexer and CDC support)
// Logical layout: physical UART buffers | virtual UART buffers | CDC uart buffers (the last two are optional)
#if defined( BUILD_SERMUX ) || defined( BUILD_USB_CDC )
static unsigned bufh_check_resnum( unsigned resid, unsigned resnum )
{
  if( ( resid == BUF_ID_UART ) && ( resnum >= SERMUX_SERVICE_ID_FIRST || resnum == CDC_UART_ID ) )
  {
    if( resnum == CDC_UART_ID )
      return resnum - CDC_UART_ID + NUM_UART + NUM_VUART_BUFS;
    else
      return resnum - SERMUX_SERVICE_ID_FIRST + NUM_UART;
  }
  else
    return resnum;
}
#define BUF_CHECK_RESNUM( resid, resnum ) resnum = bufh_check_resnum( resid, resnum )    
#else
#define BUF_CHECK_RESNUM( resid, resnum )
#endif

// Initialize the buffer of the specified resource
// resid - resource ID (BUF_ID_UART ...)
// resnum - resource number (0, 1, 2...)
// bufsize - new size of the buffer (one of the BUF_SIZE_xxx constants from
//   buf.h, or BUF_SIZE_NONE to disable buffering
// logdsize - log2(bytes) size of elements (from BUF_DSIZE_xxx constants)
// Returns 1 on success, 0 on failure
int buf_set( unsigned resid, unsigned resnum, u8 logsize, u8 logdsize )
{
  BUF_CHECK_RESNUM( resid, resnum );
  BUF_GETPTR( resid, resnum );
  
  pbuf->logdsize = logdsize;
  pbuf->logsize = logsize + logdsize;
  
  if( ( pbuf->buf = ( t_buf_data* )realloc( pbuf->buf, BUF_BYTESIZE( pbuf ) ) ) == NULL )
  {
    pbuf->logsize = BUF_SIZE_NONE;
    pbuf->rptr = pbuf->wptr = pbuf->count = 0;
    if( logsize != BUF_SIZE_NONE )
      return PLATFORM_ERR;
  }
  
  return PLATFORM_OK;
}

// Marks buffer as empty
void buf_flush( unsigned resid, unsigned resnum )
{
  BUF_CHECK_RESNUM( resid, resnum );
  BUF_GETPTR( resid, resnum );
  
  pbuf->rptr = pbuf->wptr = pbuf->count = 0;
}

// Write to buffer
// resid - resource ID (BUF_ID_UART ...)
// resnum - resource number (0, 1, 2...)
// data - pointer for where data will come from
// Returns PLATFORM_OK on success, PLATFORM_ERR on failure
// [TODO] maybe add a buffer overflow flag
int buf_write( unsigned resid, unsigned resnum, t_buf_data *data )
{
  BUF_CHECK_RESNUM( resid, resnum );
  BUF_GETPTR( resid, resnum );
  const char* s = ( const char* )data;
  char* d = ( char* )( pbuf->buf + pbuf->wptr );
  
  if( pbuf->logsize == BUF_SIZE_NONE )
    return PLATFORM_ERR;    
  if( pbuf->count > BUF_REALSIZE( pbuf ) )
  {
    fprintf( stderr, "[ERROR] Buffer overflow on resid=%d, resnum=%d, count=%d, realsize=%d!\n", resid, resnum, pbuf->count, BUF_REALSIZE( pbuf ) );
    return PLATFORM_ERR; 
  }
  DUFF_DEVICE_8( BUF_REALDSIZE( pbuf ),  *d++ = *s++ );
  
  BUF_MOD_INCR( pbuf, wptr );
    pbuf->count ++;
    
  return PLATFORM_OK;
}

// Returns 1 if the specified device is buffered, 0 otherwise
// resid - resource ID (BUF_ID_UART ...)
// resnum - resource number (0, 1, 2...)
int buf_is_enabled( unsigned resid, unsigned resnum )
{
  BUF_CHECK_RESNUM( resid, resnum );
  BUF_GETPTR( resid, resnum );
    
  return pbuf->logsize != BUF_SIZE_NONE;
}

// Return the size of the buffer in number
unsigned buf_get_size( unsigned resid, unsigned resnum )
{
  BUF_CHECK_RESNUM( resid, resnum );
  BUF_GETPTR( resid, resnum );
    
  return pbuf->logsize == BUF_SIZE_NONE ? 0 : BUF_REALSIZE( pbuf );
}

// Return the size of the data in the buffer
unsigned buf_get_count( unsigned resid, unsigned resnum )
{
  BUF_CHECK_RESNUM( resid, resnum );
  BUF_GETPTR( resid, resnum );
  
  return READ16( pbuf->count );  
}

// Get data from buffer of size dsize
// resid - resource ID (BUF_ID_UART ...)
// resnum - resource number (0, 1, 2...)
// data - pointer for where data should go
// dsize - length of data to get
// Returns PLATFORM_OK on success, PLATFORM_ERR on failure, 
//   PLATFORM_UNDERFLOW on buffer empty
int buf_read( unsigned resid, unsigned resnum, t_buf_data *data )
{
  BUF_CHECK_RESNUM( resid, resnum );
  BUF_GETPTR( resid, resnum );

  int old_status;
  const char* s = ( const char* )( pbuf->buf + pbuf->rptr );
  char* d = ( char* )data;
  
  if( pbuf->logsize == BUF_SIZE_NONE || READ16( pbuf->count ) == 0 )
    return PLATFORM_UNDERFLOW;
 
  DUFF_DEVICE_8( BUF_REALDSIZE( pbuf ),  *d++ = *s++ );

  old_status = platform_cpu_set_global_interrupts( PLATFORM_CPU_DISABLE );
  pbuf->count --;
  platform_cpu_set_global_interrupts( old_status );
  BUF_MOD_INCR( pbuf, rptr );
  
  return PLATFORM_OK;
}

#endif // #ifdef BUF_ENABLE

