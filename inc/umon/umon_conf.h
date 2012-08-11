// Monitor build configuration

#ifndef __UMON_CONF_H__
#define __UMON_CONF_H__

// Size of the call trace buffer in number of entries
// The actual memory area needed for the trace buffers
// is 9 * UMON_STACK_TRACE_ENTRIES bytes
#define UMON_STACK_TRACE_ENTRIES        100

// Size of the stack for setjmp/longjmp calls
#define UMON_SETJMP_STACK_SIZE          16

// Build statistics inside monitor
// [TODO] implement this
#define UMON_BUILD_STATS

#endif

