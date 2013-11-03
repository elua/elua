// Remote FS server

#include "net.h"
#include "remotefs.h"
#include "eluarpc.h"
#include "rfs_serial.h"
#include "server.h"
#include "type.h"
#include "log.h"
#include "os_io.h"
#include <stdio.h>
#include <limits.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "rfs.h"
#include "deskutils.h"
#include "rfs_transports.h"

#ifdef RFS_STANDALONE_MODE
int main( int argc, const char **argv )
{  
  // Initialize data
  if( rfs_init( argc, argv ) != 0 )
    return 1;
  
  // 'mem' transport doesn't work in this mode
  if( p_transport_data == &mem_transport_data )
  {
    log_err( "Invalid transport in standalone mode.\n" );
    return 1;
  }
  
  // Enter the server endless loop
  while( 1 )
  {
    p_transport_data->f_read_request();
    server_execute_request( rfs_buffer );
    p_transport_data->f_send_response();
  }

  p_transport_data->f_cleanup();
  return 0;
}
#endif
