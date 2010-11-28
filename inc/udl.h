// eLua micro dynamic loader (udl)

#ifndef __UDL_H__
#define __UDL_H__

#include "type.h"

// Binary module signature
#define UDL_MOD_SIGN          0x15AF29C8

// Maximum module name
#define UDL_MAX_MOD_NAME      16

// Error codes
#define UDL_OK                0
#define UDL_INVALID_MODULE    ( -1 )
#define UDL_OUT_OF_MEMORY     ( -2 )
#define UDL_NOT_IMPLEMENTED   ( -3 )
#define UDL_NO_SLOTS          ( -4 )
#define UDL_VERSION_ERROR     ( -5 )
#define UDL_INIT_ERROR        ( -6 )
#define UDL_CANT_UNLOAD       ( -7 )

// Module init function type
typedef int ( *p_udl_init_func )( int id );
typedef int ( *p_udl_cleanup_func )( int id );

#define UDL_MOD_INIT_FNAME    "udl_mod_init"
#define UDL_MOD_CLEANUP_FNAME "udl_mod_cleanup"

// Module cleanup function type

void udl_init();
int udl_load( const char *pname );
int udl_unload( int id );
int udl_get_id( u32 pc );
u32 udl_get_offset( int id );
u32 udl_find_symbol( int id, const char *symname );
int udl_ltr_find_next_module( int id );
void* udl_ltr_get_rotable( int id );
int udl_ltr_is_rotable( void* p );
const char* udl_get_module_name( int id );

#endif
