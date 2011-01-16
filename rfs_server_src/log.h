// Simple logging functions for the RFS server

#ifndef __LOG_H__
#define __LOG_H__

// Logging levels
#define LOG_NONE        0
#define LOG_ALL         1

void log_init( int level );
void log_msg( const char *msg, ... );
void log_err( const char* msg, ... );

#define LOG_SET_MODULE

#endif
