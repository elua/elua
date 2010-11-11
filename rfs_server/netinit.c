// Network initialization (needed under WIN32)

#ifdef WIN32_BUILD

#include <winsock2.h>
#include <windows.h>
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

#else //  #ifdef WIN32_BUILD

int net_init()
{
  return 1;
}

#endif // #ifdef WIN32_BUILD

