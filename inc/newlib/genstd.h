// stdio/stdout/stderr and generic console support

#ifndef __GENSTD_H__
#define __GENSTD_H__

#include "type.h"
#include "devman.h"

// STD device name (for devman)
#define STD_DEV_NAME         "/std"

// Send/receive function types
typedef void ( *p_std_send_char )( int fd, char c );
typedef int ( *p_std_get_char )();

// STD functions
void std_set_send_func( p_std_send_char pfunc );
void std_set_get_func( p_std_get_char pfunc );
DM_DEVICE* std_get_desc();

#endif
