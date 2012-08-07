// On-line tracing and monitoring tool for ARM

#ifdef BUILD_UMON
#include "umon.h"
#include "compilerdefs.h"

// ****************************************************************************
// Local variables and macros

static int initialized = 0;             // was the tracer initialized?
static p_umon_output output_func;
static p_umon_input input_func;
static int call_stack_idx;              // index in the call stack

// ****************************************************************************
// Local functions 

// ****************************************************************************
// Public functions

void __cyg_profile_func_enter( void *this_fn, void *call_site ) NO_INSTRUMENT;
void __cyg_profile_func_enter( void *this_fn, void *call_site )
{
  if( !initialized )
    return;
}

void __cyg_profile_func_exit( void *this_fn, void *call_site ) NO_INSTRUMENT;
void __cyg_profile_func_exit( void *this_fn, void *call_site )
{
  if( !initialized )
    return;
}

int umon_init( void *func_trace_area, unsigned func_trace_area_size,
                 const p_umon_output umon_output_fn, 
                 const p_umon_input umon_input_fn )
{
  output_func = umon_output_fn;
  input_func = umon_input_fn;
  initialized = 1;
  return UMON_OK;
}


void umon_putc( char c )
{
  if( initialized && output_func )
    output_func( c );
}

#endif // #ifdef BUILD_UMON

