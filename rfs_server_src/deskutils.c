// Utility functions for desktop programs

#include "deskutils.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>

// Secure atoi
int secure_atoi( const char *str, long *pres )
{
  char *end_ptr;
  long s1;
  
  errno = 0;
  s1 = strtol( str, &end_ptr, 10 );
  if( ( s1 == LONG_MIN || s1 == LONG_MAX ) && errno != 0 )
    return 0;
  else if( end_ptr == str )
    return 0;
  else if( s1 > INT_MAX || s1 < INT_MIN )
    return 0;
  else if( '\0' != *end_ptr )
    return 0;
  *pres = s1;
  return 1;  
}

// Local strndup function to compensate the lack of strndup in Windows
char* l_strndup( const char* s, size_t n )
{
  char* p;
  
  if( ( p = ( char* )malloc( n + 1 ) ) == NULL )
    return NULL;
  p [ 0 ] = p[ n ] = '\0';
  strncpy( p, s, n );
  return p;
} 
