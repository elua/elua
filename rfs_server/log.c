// Simple logging functions for the RFS server

#include "log.h"
#include <stdio.h>
#include <stdarg.h>

static int log_level;

void log_init( int level )
{
  log_level = level;
}

void log_msg( const char *msg, ... )
{
  va_list va;
  
  if( log_level == LOG_ALL )
  {
    va_start( va, msg );
    vprintf( msg, va );
    va_end( va );  
  }
}

void log_err( const char *msg, ... )
{
  va_list va;
  
  va_start( va, msg );
  vfprintf( stderr, msg, va );
  va_end( va );
}


