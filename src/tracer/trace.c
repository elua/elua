// On-line tracing and monitoring tool for ARM

#ifdef BUILD_TRACER
#include "tracer.h"
#include "compilerdefs.h"

// ****************************************************************************
// Local variables and macros

static int initialized = 0;             // was the tracer initialized?

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

int tracer_init( void *trace_area, unsigned max_trace_size )
{
  return TRACER_OK;
}

#endif // #ifdef BUILD_TRACER

