// Network functions (POSIX)

#include "net.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sys/select.h>
#include <unistd.h>

int net_init()
{ 
  return 1;
}

NET_SOCKET net_create_socket( int domain, int type, int protocol )
{
  return socket( domain, type, protocol ); 
}

net_ssize_t net_recvfrom( NET_SOCKET s, void *buf, size_t len, int flags, struct sockaddr* from, socklen_t *fromlen, int timeout )
{
  fd_set fds;
  struct timeval tv;
  
  FD_ZERO( &fds );
  FD_SET( s, &fds );
  tv.tv_sec = timeout / 1000000;
  tv.tv_usec = ( timeout % 1000000 ) * 1000;
  if( select( s + 1, &fds, NULL, NULL, timeout == NET_INF_TIMEOUT ? NULL : &tv ) <= 0 )
    return 0;
  return recvfrom( s, buf, len, flags, from, fromlen );      
}

net_ssize_t net_sendto( NET_SOCKET s, const void *buf, size_t len, int flags, const struct sockaddr *to, socklen_t tolen )
{
  return sendto( s, buf, len, flags, to, tolen );
}

int net_close( NET_SOCKET s )
{
  return close( s );
}

net_sync_object net_get_sync_object( NET_SOCKET s )
{
  return s;
}
