// eLua interrupt handlers support

#ifndef __ELUA_INT_H__
#define __ELUA_INT_H__

#include "type.h"

typedef u8 elua_int_id;
typedef u8 elua_int_resnum;

#define ELUA_INT_EMPTY_SLOT             0
#define ELUA_INT_FIRST_ID               1
#define ELUA_INT_INVALID_INTERRUPT      0xFF

// This is what gets pushed in the interrupt queue
typedef struct 
{
  elua_int_id id;
  elua_int_resnum resnum;
} elua_int_element;

// Handler key in the registry
#define LUA_INT_HANDLER_KEY             ( int )&elua_int_add

// Maximum number of interrupt sources that can appear in ANY backend
// Must be a multiple of 32
#define LUA_INT_MAX_SOURCES             128

// Function prototypes
int elua_int_add( elua_int_id inttype, elua_int_resnum resnum );
void elua_int_enable( elua_int_id inttype );
void elua_int_disable( elua_int_id inttype );
int elua_int_is_enabled( elua_int_id inttype );
void elua_int_disable_all();

#endif

