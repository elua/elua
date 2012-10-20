// Memory tracer implementation

#include <stdio.h>
#include <string.h>
#include <reent.h>
#include "memtrace.h"
#include "platform_conf.h"
#include "umon.h"

#ifdef ENABLE_MEM_TRACER

////////////////////////////////////////////////////////////////////////////////

static p_memt_output mt_out;
static u8 mt_level;

void* __real__malloc_r( struct _reent* r, size_t size );
void* __real__calloc_r( struct _reent* r, size_t nelem, size_t elem_size );
void __real__free_r( struct _reent* r, void* ptr );
void* __real__realloc_r( struct _reent* r, void* ptr, size_t size );

static void mth_putc( u8 data )
{
  if( mt_out && ( mt_level > 0 ) )
    mt_out( data );
}

static void mth_u8( u8 data )
{
  mth_putc( data );
}

static void mth_u16( u16 data )
{
  mth_u8( data >> 8 );
  mth_u8( data & 0xFF );
}

static void mth_u32( u32 data )
{
  mth_u16( data >> 16 );
  mth_u16( data & 0xFFFF );
}

static void mth_puts( const char *pdata )
{
  if( NULL == pdata )
    pdata = "";
  mth_u16( strlen( pdata ) );
  while( *pdata )
    mth_putc( *pdata ++ );
}

static void mth_op( u8 op, int has_trace )
{
  int depth;
  unsigned to, from;

  mth_u8( op );
  if( has_trace )
  {
    // Include a stack trace in this log
    mth_u8( mt_level | MT_STACK_TRACE_MASK );
    depth = umon_get_stack_depth();
    mth_u16( depth );
    while( --depth >= 0 )
    {
      umon_get_trace_entry( depth, &to, &from );
      mth_u16( from );
      mth_u16( to );
    }
  }
  else
    mth_u8( mt_level );
}

////////////////////////////////////////////////////////////////////////////////

void mt_init( p_memt_output outf )
{
  mt_out = outf;
  mt_level = 1;
}

void mt_start( const char *pdata )
{
  mt_level ++;
  mth_op( MT_START, MT_WITHOUT_TRACE );
  mth_puts( pdata );
}

void mt_stop()
{
  mth_op( MT_STOP, MT_WITHOUT_TRACE );
  mt_level --;
}

void mt_trace_malloc( const void* ptr, unsigned len )
{
  mth_op( MT_MALLOC, MT_WITHOUT_TRACE );
  mth_u32( ( u32 )ptr );
  mth_u32( ( u32 )len );
}

void mt_trace_calloc( const void *ptr, unsigned nmemb, unsigned size )
{
  mth_op( MT_CALLOC, MT_WITHOUT_TRACE );
  mth_u32( ( u32 )ptr );
  mth_u32( ( u32 )nmemb );
  mth_u32( ( u32 )size );
}

void mt_trace_free( const void *ptr )
{
  mth_op( MT_FREE, NULL == ptr ? MT_WITH_TRACE : MT_WITHOUT_TRACE );
  mth_u32( ( u32 )ptr );
}

void mt_trace_realloc( const void *newptr, const void *oldptr, unsigned len )
{
  mth_op( MT_REALLOC, MT_WITHOUT_TRACE );
  mth_u32( ( u32 )newptr );
  mth_u32( ( u32 )oldptr );
  mth_u32( ( u32 )len );
}

////////////////////////////////////////////////////////////////////////////////

void* __wrap__malloc_r( struct _reent* r, size_t size )
{
  void *ptr = __real__malloc_r( r, size );

  mt_trace_malloc( ptr, size );
  return ptr;
}

void* __wrap__calloc_r( struct _reent* r, size_t nelem, size_t elem_size )
{
  void *ptr = __real__calloc_r( r, nelem, elem_size );

  mt_trace_calloc( ptr, nelem, elem_size );
  return ptr;
}

void __wrap__free_r( struct _reent* r, void* ptr )
{
  mt_trace_free( ptr );
  __real__free_r( r, ptr );
}

void* __wrap__realloc_r( struct _reent* r, void* ptr, size_t size )
{
  void *res = __real__realloc_r( r, ptr, size );

  mt_trace_realloc( res, ptr, size );
  return res;
}

#endif // #ifdef ENABLE_MEM_TRACER

