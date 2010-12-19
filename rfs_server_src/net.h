// Network functions

#ifndef __NETINIT_H__
#define __NETINIT_H__

typedef int net_ssize_t;

#ifdef WIN32_BUILD

#include <winsock2.h>
#include <windows.h>
typedef int socklen_t;
#define INVALID_SOCKET_VALUE  NULL
typedef struct 
{
  SOCKET s;
  OVERLAPPED o;
} NET_DATA;
typedef NET_DATA* NET_SOCKET;
#define net_socket( d )       ( d )->s
typedef HANDLE net_sync_object;

#else // #ifdef WIN32_BUILD

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/select.h>
typedef int NET_SOCKET;
#define INVALID_SOCKET_VALUE  ( -1 )
#define net_socket( s )       s
typedef int net_sync_object;
#endif // #ifdef WIN32_BUILD

#define NET_INF_TIMEOUT       ( -1 )

int net_init();
NET_SOCKET net_create_socket( int domain, int type, int protocol );
net_ssize_t net_recvfrom( NET_SOCKET s, void *buf, size_t len, int flags, struct sockaddr* from, socklen_t *fromlen, int timeout );
net_ssize_t net_sendto( NET_SOCKET s, const void *buf, size_t len, int flags, const struct sockaddr *to, socklen_t tolen );
int net_close( NET_SOCKET s );
net_sync_object net_get_sync_object( NET_SOCKET s );

#endif
