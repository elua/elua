// Utility functions for desktop programs

#ifndef __DESKUTILS_H__
#define __DESKUTILS_H__

#include "type.h"
#include <stddef.h>

int secure_atoi( const char *str, long *pres );
char* l_strndup( const char* s, size_t n );

#endif
