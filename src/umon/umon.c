// On-line tracing and monitoring tool for ARM

#ifdef BUILD_UMON

#include "umon.h"
#include "compilerdefs.h"
#include "umon_utils.h"
#include <platform.h>

// ****************************************************************************
// Local variables and macros

static int initialized = 0;             // was the tracer initialized?
static p_umon_output output_func;
static p_umon_input input_func;
static unsigned *p_call_stack;          // pointer to the call stack
static unsigned *p_call_stack_crt;      // pointer to current element in the stack
static int call_stack_idx;              // index in the call stack
static int stack_depth_break_idx;       // breakpoint if this stack depth is reached

// ****************************************************************************
// Local functions 

// ****************************************************************************
// Public functions

void __cyg_profile_func_enter( void *this_fn, void *call_site ) NO_INSTRUMENT;
void __cyg_profile_func_enter( void *this_fn, void *call_site )
{
  if( !initialized )
    return;
  *p_call_stack_crt ++ = ( unsigned )this_fn & ~1;
  *p_call_stack_crt ++ = ( unsigned )call_site & ~1;
  call_stack_idx ++;
  if( stack_depth_break_idx >= call_stack_idx )
  {
    umon_trace_start();
    umon_printf( "[UMON] Breakpoint on call stack depth limit (%d)\n", call_stack_idx );
    umon_print_stack_trace();
    while( 1 );
    umon_trace_end();
  }
}

void __cyg_profile_func_exit( void *this_fn, void *call_site ) NO_INSTRUMENT;
void __cyg_profile_func_exit( void *this_fn, void *call_site )
{
  if( !initialized || call_stack_idx == 0 )
    return;
  p_call_stack_crt -= 2;
  if( p_call_stack_crt[ 0 ] != ( ( unsigned )this_fn & ~1 ) )
  {
    umon_printf( "ERROR: this_fn_stack = %X, this_fn_par = %X\n", p_call_stack_crt[ 0 ], ( unsigned )this_fn & ~1 );
    while( 1 );
  }
  if( p_call_stack_crt[ 1 ] != ( ( unsigned )call_site & ~1 ) )
  {
    umon_printf( "ERROR: call_site_stack = %X, call_site_par = %X\n", p_call_stack_crt[ 1 ], ( unsigned )call_site & ~1 );
    while( 1 );
  }
  call_stack_idx --;
}

int umon_trace_start()
{
  platform_cpu_set_global_interrupts( PLATFORM_CPU_DISABLE );
  return call_stack_idx;
}

void umon_get_trace_entry( unsigned idx, unsigned *pto, unsigned *pfrom )
{
  idx <<= 1;
  if( pto )
    *pto = p_call_stack[ idx ];
  if( pfrom )
    *pfrom = p_call_stack[ idx + 1 ];
}

void umon_trace_end()
{
  platform_cpu_set_global_interrupts( PLATFORM_CPU_ENABLE );
}

int umon_init( void *call_stack,
                 const p_umon_output umon_output_fn, 
                 const p_umon_input umon_input_fn )
{
  output_func = umon_output_fn;
  input_func = umon_input_fn;
  call_stack_idx = 0;
  stack_depth_break_idx = UMON_STACK_BREAK_DISABLE;
  p_call_stack = p_call_stack_crt = ( unsigned* )call_stack;
  initialized = 1;
  return UMON_OK;
}

const char* umon_get_function_name( unsigned addr ) 
{
  unsigned temp;
  const char* s;

  addr = ( addr & ~1 ) - 4;
  temp = *( unsigned* )addr;
  if( ( temp >> 24 ) != 0xFF )
    return NULL;
  temp = temp & 0xFF;
  s = ( const char* )( addr - temp );
  // Additional check for an actual function name at the computed address
  if( ( ( addr - temp + umon_strlen( s ) + 4 ) & ~3 ) != addr )
    return NULL;
  return s;
}

void umon_print_stack_trace()
{
  int depth = call_stack_idx;
  unsigned to, from;
  const char *fname, *fromname;
  unsigned last_to = 0;

  while( --depth >= 0 )
  {
    umon_get_trace_entry( depth, &to, &from );
    if( ( fname = umon_get_function_name( to ) ) == NULL )
      fname = "<UNKNOWN>";
    if( depth == 0 )
    {
      fromname = "<UNKNOWN>";
      last_to = from;
    }
    else
    {
      umon_get_trace_entry( depth - 1, &last_to, NULL );
      fromname = umon_get_function_name( last_to );
    }
    umon_printf( "%s (%8X) <- %s+%X (%8X))\n", fname, to, fromname, from - last_to, from );
  }
}

void umon_break_on_stack_depth( unsigned depth )
{
  stack_depth_break_idx = depth;
}

void umon_putc( char c )
{
  if( initialized && output_func )
    output_func( c );
}

#endif // #ifdef BUILD_UMON

