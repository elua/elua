// Network services provided by eLua

#ifndef __ELUA_NET_H__
#define __ELUA_NET_H__

#include "type.h"

// eLua network typedefs
typedef s16 elua_net_size;

// eLua services ports
#define ELUA_NET_TELNET_PORT          23

// Different constants
#define ELUA_NET_SOCK_STREAM          0
#define ELUA_NET_SOCK_DGRAM           1

// eLua TCP/IP functions
int elua_net_socket( int type );

int elua_net_close( int s );
elua_net_size elua_net_recv( int s, void* buf, elua_net_size maxsize );
elua_net_size elua_net_send( int s, const void* buf, elua_net_size len );
int elua_accept( u16 port );

int elua_net_get_last_err( int s );
int elua_net_get_telnet_socket();

#endif
