// Remote filesystem server implementation

#ifndef __SERVER_H__
#define __SERVER_H__

#include "type.h"

// Error codes
#define SERVER_OK     0
#define SERVER_ERR    1

// Server function                     
void server_setup( const char *basedir );
void server_cleanup();
int server_execute_request( u8 *pdata );

#endif
