// On-line micro monitor for ARM cores

#ifndef __UMON_H__
#define __UMON_H__

#include "compilerdefs.h"

// Console input/output functions
typedef void ( *p_umon_output )( char );
typedef int ( *p_umon_input )( void );

// Error codes
#define UMON_OK               0

// Public functions
int umon_init( void *func_trace_area, unsigned func_trace_area_size,
                 const p_umon_output umon_output_fn, 
                 const p_umon_input umon_input_fn ) NO_INSTRUMENT;
void umon_putc( char c ) NO_INSTRUMENT;

#endif // #ifndef __TRACER_H__

