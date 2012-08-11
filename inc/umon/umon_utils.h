// Various utility functions for the monitor

#ifndef __UMON_UTILS_H__
#define __UMON_UTILS_H__

#include "umon.h"
#include "compilerdefs.h"

void umon_printf( const char *fmt, ... ) NO_INSTRUMENT;
int umon_strlen( const char *s ) NO_INSTRUMENT;
// The next functions are imlemented in umon_asm.s
void umon_enable_ints();
void umon_disable_ints();
int umon_get_int_stat_and_disable();
unsigned umon_get_current_int();

#endif // #ifndef __UMON_UTILS_H__

