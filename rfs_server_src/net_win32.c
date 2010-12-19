// Network functions (WIN32)

#include "net.h"
#include <stdlib.h>
#include <string.h>
#include "log.h"

int net_init()
{
  // The socket subsystem must be initialized if working in Windows
  WORD wVersionRequested;
  WSADATA wsaData;
  int err;
 
  wVersionRequested = MAKEWORD( 2, 0 );
  err = WSAStartup( wVersionRequested, &wsaData );  
  if( err != 0 )
  {
    log_err( "Unable to initialize the socket subsystem\n" );
    return 0;
  }  
  return 1;
}

NET_SOCKET net_create_socket( int domain, int type, int protocol )
{
  SOCKET s;
  NET_SOCKET d;
  
  if( ( s = WSASocket( domain, type, protocol, NULL, 0, WSA_FLAG_OVERLAPPED ) ) == INVALID_SOCKET )
    return INVALID_SOCKET_VALUE;
  if( ( d = malloc( sizeof( NET_DATA ) ) ) == NULL )
  {
    closesocket( s );
    return INVALID_SOCKET_VALUE;
  } 
  memset( d, 0, sizeof( NET_DATA ) );
  d->s = s;
  if( ( d->o.hEvent = CreateEvent( NULL, TRUE, FALSE, NULL ) ) == NULL )
  {
    closesocket( s );
    return INVALID_SOCKET_VALUE;
  }
  return d;  
}

net_ssize_t net_recvfrom( NET_SOCKET s, void *buf, size_t len, int flags, struct sockaddr* from, socklen_t *fromlen, int timeout )
{
  DWORD readbytes = 0;
  DWORD rflags = ( DWORD )flags;
  BOOL dwRes;
  DWORD selflags;
  WSABUF datadesc = { len, buf };
  
  ResetEvent( s->o.hEvent );
  if( WSARecvFrom( s->s, &datadesc, 1, &readbytes, &rflags, from, fromlen, &s->o, NULL ) == SOCKET_ERROR )
  {
    if( WSAGetLastError() != WSA_IO_PENDING )
      return 0;
  }
  else
    return readbytes;
  
  dwRes = WaitForSingleObject( s->o.hEvent, timeout == NET_INF_TIMEOUT ? INFINITE : timeout );
  if( dwRes == WAIT_OBJECT_0 ) 
  {
    if( !WSAGetOverlappedResult( s->s, &s->o, &readbytes, TRUE, &selflags ) )
      readbytes = 0;
  }
  else if( dwRes == WAIT_TIMEOUT )
  {
    WSAGetOverlappedResult( s->s, &s->o, &readbytes, TRUE, &selflags );
    readbytes = 0;
  }
  
  return readbytes;    
}

net_ssize_t net_sendto( NET_SOCKET s, const void *buf, size_t len, int flags, const struct sockaddr *to, socklen_t tolen )
{
  DWORD wrotebytes;
  WSABUF datadesc = { len, ( char* )buf };
    
  if( WSASendTo( s->s, &datadesc, 1, &wrotebytes, flags, to, tolen, NULL, NULL ) == SOCKET_ERROR )
    return 0;
  return wrotebytes;
}

int net_close( NET_SOCKET s )
{
  closesocket( s->s );
  CloseHandle( s->o.hEvent );
  free( s );
  return 0;  
}

net_sync_object net_get_sync_object( NET_SOCKET s )
{
  return s->o.hEvent;
}


