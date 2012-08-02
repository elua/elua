// Compiler-specific definitions

#ifndef __COMPILERDEFS_H__
#define __COMPILERDEFS_H__

#ifdef __GNUC__ // GNU compiler
#define NO_INSTRUMENT  __attribute__((no_instrument_function))
#else // #ifdef
#error Currently, the tracer compilers only with GCC based compilers
#endif // #ifdef __GNUC__

#endif // #ifndef __COMPILERDEFS_H__

