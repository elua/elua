// On-line micro monitor for ARM cores

#ifndef __UMON_H__
#define __UMON_H__

#include "compilerdefs.h"

// Console input/output functions
typedef void ( *p_umon_output )( char );
typedef int ( *p_umon_input )( void );

// Various constants
#define UMON_STACK_BREAK_DISABLE        ( -1 )

// Error codes
#define UMON_OK                         0
#define UMON_ERR_STACK_OVERFLOW         1

// uMON data types
typedef unsigned long long um_u64;
typedef long long um_s64;
typedef unsigned int um_u32;
typedef int um_s32;
typedef unsigned short um_u16;
typedef short um_s16;
typedef unsigned char um_u8;
typedef char um_s8;

// Public functions
void umon_set_output_func( p_umon_output outfunc );
void umon_set_input_func( p_umon_input infunc );
// Breakpoint functions
int umon_break_on_stack_depth( unsigned depth ) NO_INSTRUMENT;
// Trace-related functions               
int umon_trace_start() NO_INSTRUMENT;
int umon_get_stack_depth() NO_INSTRUMENT;
void umon_get_trace_entry( unsigned idx, unsigned *pto, unsigned *pfrom ) NO_INSTRUMENT;
void umon_trace_end( int stat ) NO_INSTRUMENT;
const char* umon_get_function_name( unsigned addr ) NO_INSTRUMENT;
void umon_print_stack_trace() NO_INSTRUMENT;
// setjmp/longjmp handling
void umon_handle_setjmp( unsigned jmpbuf ) NO_INSTRUMENT;
void umon_handle_longjmp( unsigned jmpbuf ) NO_INSTRUMENT;
// Generic utilities
void umon_putc( char c ) NO_INSTRUMENT;

#endif // #ifndef __TRACER_H__

