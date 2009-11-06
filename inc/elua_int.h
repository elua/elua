// eLua interrupt handlers support

#ifndef __ELUA_INT_H__
#define __ELUA_INT_H__

#include "type.h"

typedef u8 elua_int_id;

// Handler key in the registry
#define LUA_INT_HANDLER_KEY   ( int )&elua_int_add

// Function prototypes
int elua_int_add( elua_int_id inttype );

#endif

