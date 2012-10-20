// On-line tracing and monitoring tool for ARM

#ifdef BUILD_UMON

#include "umon.h"
#include "compilerdefs.h"
#include "umon_utils.h"
#include "umon_conf.h"
#include <setjmp.h>

// ****************************************************************************
// Local variables and macros

#ifndef NULL
#define NULL                            ( void* )0
#endif

#define umon_restore_ints( stat )       do {\
  if( stat )\
    umon_enable_ints(); \
  } while( 0 )

static p_umon_output output_func;
static p_umon_input input_func;
static int call_stack_idx;                                          // index in the call stack
static int stack_depth_break_idx = UMON_STACK_BREAK_DISABLE;        // breakpoint if this stack depth is reached
static unsigned call_stack[ 2 * UMON_STACK_TRACE_ENTRIES ];         // stack trace area
static unsigned char call_stack_int[ UMON_STACK_TRACE_ENTRIES ];    // interrupt for each call stack level
static unsigned *p_call_stack_crt = call_stack;                     // pointer to current element in the stack
// setjmp/longjmp handling
typedef struct 
{
  unsigned pjmpbuf;
  int call_stack_idx;
} UMON_SETJMP_DATA;
static UMON_SETJMP_DATA setjmp_stack[ UMON_SETJMP_STACK_SIZE ];
static int setjmp_stack_idx;       // index in the setjmp stack

extern int __real_setjmp( jmp_buf );
extern void __real_longjmp( jmp_buf, int );

// ****************************************************************************
// Local functions 

// ****************************************************************************
// Public functions

void __cyg_profile_func_enter( void *this_fn, void *call_site ) NO_INSTRUMENT;
void __cyg_profile_func_enter( void *this_fn, void *call_site )
{
  int ints_on = umon_get_int_stat_and_disable();

  // setjmp/longjmp are handled separately
  // [TODO] is this really needed? After all, setjmp won't appear in the stack trace if we do this...
//  if( this_fn == &__real_setjmp || this_fn == &__real_longjmp )
//    goto fexit;
  if( stack_depth_break_idx >= call_stack_idx )
  {
    umon_printf( "[UMON] Breakpoint on call stack depth limit (%d)\n", call_stack_idx );
    umon_print_stack_trace();
    // [TODO] proper entry/exit for this breakpoint condition from this
    while( 1 );
  }
  if( call_stack_idx >= UMON_STACK_TRACE_ENTRIES )
  {
    umon_printf( "[UMON] call stack trace overflow, system halted.\n" );
    while( 1 );
  }
  *p_call_stack_crt ++ = ( unsigned )this_fn & ~1;
  *p_call_stack_crt ++ = ( unsigned )call_site & ~1;
  call_stack_int[ call_stack_idx ] = umon_get_current_int();
  call_stack_idx ++;
  umon_restore_ints( ints_on );
}

void __cyg_profile_func_exit( void *this_fn, void *call_site ) NO_INSTRUMENT;
void __cyg_profile_func_exit( void *this_fn, void *call_site )
{
  int ints_on = umon_get_int_stat_and_disable();

  if( call_stack_idx == 0 ) 
    goto fexit;
  // setjmp/longjmp are handled separately
  // [TODO] is this really needed? After all, setjmp won't appear in the stack trace if we do this...
//  if( this_fn == &__real_setjmp || this_fn == &__real_longjmp )
//    goto fexit;
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
fexit:
  umon_restore_ints( ints_on );
}

void umon_handle_setjmp( unsigned addr )
{
  int ints_on = umon_get_int_stat_and_disable();
  // [TODO] check stack idx for overflow
  setjmp_stack[ setjmp_stack_idx ].pjmpbuf = addr;
  setjmp_stack[ setjmp_stack_idx ].call_stack_idx = call_stack_idx;
  setjmp_stack_idx ++;
  umon_restore_ints( ints_on );
}

void umon_handle_longjmp( unsigned addr )
{
  int ints_on = umon_get_int_stat_and_disable();
  // [TODO] check stack idx for underflow
  while( setjmp_stack[ --setjmp_stack_idx ].pjmpbuf != addr );
  call_stack_idx = setjmp_stack[ setjmp_stack_idx ].call_stack_idx;
  p_call_stack_crt = call_stack + 2 * call_stack_idx;
  umon_restore_ints( ints_on );
}

int umon_trace_start()
{
  return umon_get_int_stat_and_disable();
}

int umon_get_stack_depth()
{
  return call_stack_idx;  
}

void umon_get_trace_entry( unsigned idx, unsigned *pto, unsigned *pfrom )
{
  idx <<= 1;
  if( pto )
    *pto = call_stack[ idx ];
  if( pfrom )
    *pfrom = call_stack[ idx + 1 ];
}

void umon_trace_end( int stat )
{
  umon_restore_ints( stat );
}

void umon_set_output_func( p_umon_output outfunc )
{
  output_func = outfunc;
}

void umon_set_input_func( p_umon_input infunc )
{
  input_func = infunc;
}

const char* umon_get_function_name( unsigned addr ) 
{
  unsigned temp;
  const char* s;

  // From the GCC manual: -mpoke-function-name
  // Write the name of each function into the text section, directly preceding the function prologue.  The generated code is similar to this:
  // t0
  //   .ascii "arm_poke_function_name", 0
  //   .align
  // t1
  //   .word 0xff000000 + (t1 - t0)
  // arm_poke_function_name
  //   mov     ip, sp
  //   stmfd   sp!, {fp, ip, lr, pc}
  //   sub     fp, ip, #4

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
  unsigned to, from, temp;
  const char *fname;

  while( --depth >= 0 )
  {
    umon_get_trace_entry( depth, &to, &from );
    fname = "<UNKNOWN>";
    umon_printf( "[%2d] ", depth );
    if( call_stack_int[ depth ] != 0 )
      umon_printf( "{%2X} ", call_stack_int[ depth ] );
    umon_printf( "%s (%8X) <- %8X\n", fname, to, from );
  }
}

int umon_break_on_stack_depth( unsigned depth )
{
  if( depth >= UMON_STACK_TRACE_ENTRIES )
    return UMON_ERR_STACK_OVERFLOW;
  stack_depth_break_idx = depth;
  return UMON_OK;
}

void umon_putc( char c )
{
  if( output_func )
    output_func( c );
}

#endif // #ifdef BUILD_UMON

