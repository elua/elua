// On-line tracing and monitoring tool for ARM

#ifndef __TRACER_H__
#define __TRACER_H__

// Error codes
#define TRACER_OK             0

// Public functions
int tracer_init( void *trace_area, unsigned max_trace_size );

#endif // #ifndef __TRACER_H__
