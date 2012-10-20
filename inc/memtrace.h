// Memory tracer interface

#ifndef __MEMTRACE_H__
#define __MEMTRACE_H__

#include "type.h"
#include "umon.h"

// Operation types
enum
{
  MT_START = 0,
  MT_MALLOC,
  MT_FREE,
  MT_REALLOC,
  MT_CALLOC,
  MT_STOP
};

#define MT_STACK_TRACE_MASK   128
#define MT_WITHOUT_TRACE      0
#define MT_WITH_TRACE         1

// Mem tracer output function
typedef void ( *p_memt_output )( u8 );

void mt_init( p_memt_output outf );
void mt_start( const char *pdata );
void mt_stop();
void mt_trace_malloc( const void* ptr, unsigned len ) NO_INSTRUMENT;
void mt_trace_calloc( const void *ptr, unsigned nmemb, unsigned size ) NO_INSTRUMENT;
void mt_trace_free( const void *ptr ) NO_INSTRUMENT;
void mt_trace_realloc( const void *newptr, const void *oldptr, unsigned len ) NO_INSTRUMENT;

#endif // #ifndef __MEMTRACE_H__

