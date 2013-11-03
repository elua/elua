// eLua interrupt handlers support

#ifndef __ELUA_INT_H__
#define __ELUA_INT_H__

#include "type.h"

typedef u8 elua_int_id;
// The resource number is a 16-bit integer because it must be able to hold a 
// port/pin combination coded as specified in platform.h 
typedef u16 elua_int_resnum;

#define ELUA_INT_EMPTY_SLOT             0
#define ELUA_INT_FIRST_ID               1
#define ELUA_INT_INVALID_INTERRUPT      0xFF

// This is what gets pushed in the interrupt queue
typedef struct 
{
  elua_int_id id;
  elua_int_resnum resnum;
} elua_int_element;

// Interrupt functions and descriptor
typedef int ( *elua_int_p_set_status )( elua_int_resnum resnum, int state ); 
typedef int ( *elua_int_p_get_status )( elua_int_resnum resnum );
typedef int ( *elua_int_p_get_flag )( elua_int_resnum resnum, int clear );
typedef struct 
{
  elua_int_p_set_status int_set_status;
  elua_int_p_get_status int_get_status;
  elua_int_p_get_flag int_get_flag;
} elua_int_descriptor;

// C interrupt handlers
typedef void( *elua_int_c_handler )( elua_int_resnum resnum );

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
void elua_int_cleanup(void);
void elua_int_disable_all(void);
elua_int_c_handler elua_int_set_c_handler( elua_int_id inttype, elua_int_c_handler phandler );
elua_int_c_handler elua_int_get_c_handler( elua_int_id inttype );

#endif

