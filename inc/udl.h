// eLua micro dynamic loader (udl)

#ifndef __UDL_H__
#define __UDL_H__

#include "type.h"

// Binary module signature
#define UDL_MOD_SIGN          0x15AF29C8

// Error codes
#define UDL_OK                0
#define UDL_INVALID_MODULE    ( -1 )
#define UDL_OUT_OF_MEMORY     ( -2 )
#define UDL_NOT_IMPLEMENTED   ( -3 )
#define UDL_NO_SLOTS          ( -4 )

void udl_init();
u32 udl_get_elua_symbol( u32 idx );
int udl_load( const char *pname );
int udl_unload( int id );
int udl_get_id( u32 pc );
u32 udl_get_offset( int id );
u32 udl_find_symbol( int id, const char *symname );

#endif
