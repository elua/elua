// Service multiplexer 

#include "net.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include "config.h"
#include "log.h"
#include "type.h"
#include "serial.h"
#include "sermux.h"
#include "rfs.h"

// ****************************************************************************
// Data structures and local variables

#define MODE_MUX              1
#define MODE_RFSMUX           2

// Send/receive/init function pointers
typedef u32 ( *p_recv_func )( u8 *p, u32 size );
typedef u32 ( *p_send_func )( const u8 *p, u32 size );
typedef int ( *p_init_func )( void );

// Serial thread buffer structure
typedef struct {
  const char *pname;
  ser_handler fd;
} SERVICE_DATA;

// Serial transport data structure
typedef struct {
  ser_handler fd;
  const char *pname;
  long speed;
} TRANSPORT_SER;

static SERVICE_DATA *services;
static unsigned vport_num;

static TRANSPORT_SER *transport_data;
static p_recv_func transport_recv;
static p_send_func transport_send;
static p_init_func transport_init;

static int service_id_in = -1, service_id_out = -1;
 
static ser_handler transport_hnd = SER_HANDLER_INVALID;
static int mux_mode;
static int verbose_mode;

// ***************************************************************************
// Serial transport implementation

static u32 transport_ser_recv( u8 *p, u32 size ) 
{
  TRANSPORT_SER *pser = ( TRANSPORT_SER* )transport_data;

  return ser_read( pser->fd, p, size, SER_NO_TIMEOUT );
}

static u32 transport_ser_send( const u8 *p, u32 size )
{
  TRANSPORT_SER *pser = ( TRANSPORT_SER* )transport_data;

  return ser_write( pser->fd, p, size );
}

static int transport_ser_init()
{
  TRANSPORT_SER *pser = ( TRANSPORT_SER* )transport_data;

  if( ( pser->fd = ser_open( pser->pname ) ) == ( ser_handler )-1 )
  {
    log_err( "Unable to open %s\n", pser->pname );
    return 0;
  }
  if( ser_setup( pser->fd, pser->speed, 8, SER_PARITY_NONE, SER_STOPBITS_1 ) != SER_OK )
  {
    log_err( "Unable to setup serial port %s\n", pser->pname );
    return 0;
  }
  transport_hnd = pser->fd;
  while( ser_read_byte( pser->fd, SER_NO_TIMEOUT ) != -1 );
  log_msg( "Running serial transport on port %s at %u baud (8N1)\n", pser->pname, ( unsigned )pser->speed );
  return 1;
}

// ****************************************************************************
// Utility functions and helpers

static void transport_send_byte( u8 data )
{
  transport_send( &data, 1 );
}

static int transport_read_byte() 
{
  u8 c;

  return transport_recv( &c, 1 ) == 1 ? c : -1;
}

// Secure atoi
static int secure_atoi( const char *str, long *pres )
{
  char *end_ptr;
  long s1;
  
  errno = 0;
  s1 = strtol( str, &end_ptr, 10 );
  if( ( s1 == LONG_MIN || s1 == LONG_MAX ) && errno != 0 )
    return 0;
  else if( end_ptr == str )
    return 0;
  else if( s1 > INT_MAX || s1 < INT_MIN )
    return 0;
  else if( '\0' != *end_ptr )
    return 0;
  *pres = s1;
  return 1;  
}

// Local strndup function to compensate the lack of strndup in Windows
static char* l_strndup( const char* s, size_t n )
{
  char* p;
  
  if( ( p = ( char* )malloc( n + 1 ) ) == NULL )
    return NULL;
  p [ 0 ] = p[ n ] = '\0';
  strncpy( p, s, n );
  return p;
} 

// Transport parser
static int parse_transport( const char* s )
{
  const char *c;
  static TRANSPORT_SER tser; 

  if( ( c = strchr( s, ',' ) ) == NULL )
  {
    log_err( "Invalid serial transport syntax.\n" );
    return 0;
  }
  if( secure_atoi( c + 1, &tser.speed ) == 0 )
  {
    log_err( "Invalid port speed\n" );
    return 0;
  }
  tser.pname = l_strndup( s, c - s );
  transport_data = &tser;
  transport_send = transport_ser_send;
  transport_recv = transport_ser_recv;
  transport_init = transport_ser_init;
  return 1;
}

// ****************************************************************************
// Program entry point

#define MODE_IDX              1
#define MAIN_TRANSPORT_IDX    2
#define FIRST_SERVICE_IDX     3
#define MIN_ARGC_COUNT        4

int main( int argc, char **argv )
{
  unsigned i;
  SERVICE_DATA *tservice;
  int c;
  int temp, sdata;
  int got_esc = 0;
  fd_set fds;
  char* rfs_dir_name;

  // Interpret arguments
  setvbuf( stdout, NULL, _IONBF, 0 );  
  if( argc < MIN_ARGC_COUNT )
  {
    log_err( "Usage: %s <mode> <transport> <vcom1> [<vcom2>] ... [<vcomn>] [-v]\n", argv[ 0 ] );
    log_err( "  mode: 'mux' for multiplexer mode, 'rfsmux:<directory>' for combined RFS and multiplexer mode.\n" );
    log_err( "  transport: '<port>,<baud>.\n" );
    log_err( "  vcom1, ..., vcomn: multiplexer serial ports." );
    log_err( "  Use '-v' for verbose output.\n" );
    return 1;
  }
  
  // Check mode
  if( !strcmp( argv[ MODE_IDX ], "mux" ) )
    mux_mode = MODE_MUX;
  else if( !strncmp( argv[ MODE_IDX ], "rfsmux:", strlen( "rfsmux:" ) ) )
  {
    rfs_dir_name = argv[ MODE_IDX ] + strlen( "rfsmux:" );
    mux_mode = MODE_RFSMUX;
  }
  else
  {
    log_err( "Invalid mode.\n" );
    return 1;
  } 
  
  // Check verbose
  i = argc - 1;
  if( !strcasecmp( argv[ i ], "-v" ) )
  {
    i --;
    log_init( LOG_ALL );
    verbose_mode = 1;
  }
  else
    log_init( LOG_NONE );
  
  // Get number of virtual UARTs     
  if( ( vport_num = i - FIRST_SERVICE_IDX + 1 ) > SERMUX_SERVICE_MAX )
  {
    log_err( "Too many service ports, maximum is %d\n", SERMUX_SERVICE_MAX );
    return 1;
  }
  
  // Parse transport data and initialize it
  if( parse_transport( argv[ MAIN_TRANSPORT_IDX ] ) == 0 )
    return 1;
  if( transport_init() == 0 )
    return 1;

  // Open all the service ports
  if( ( services = ( SERVICE_DATA* )malloc( sizeof( SERVICE_DATA ) * vport_num ) ) == NULL )
  {
    log_err( "Not enough memory\n" );
    return 1;
  }

  memset( services, 0, sizeof( SERVICE_DATA ) * vport_num );
  for( i = 0; i < vport_num; i ++ ) 
  {
    tservice = services + i;
    if( ( tservice->fd = ser_open( argv[ i + FIRST_SERVICE_IDX ] ) ) == SER_HANDLER_INVALID )
    {
      log_err( "Unable to open port %s\n", argv[ i + FIRST_SERVICE_IDX ] );
      return 1;
    }
    if( ser_setup( tservice->fd, transport_data->speed, SER_DATABITS_8, SER_PARITY_NONE, SER_STOPBITS_1 ) != SER_OK )
    {
      log_err( "Unable to setup serial port %s\n", argv[ i + FIRST_SERVICE_IDX ] );
      return 1;
    }
    tservice->pname = argv[ i + FIRST_SERVICE_IDX ];
  }
  
  // Setup RFS server in RFSMUX mode
  if( mux_mode == MODE_RFSMUX )
  {
    char *args[] = { "dummy", "mem", rfs_dir_name, NULL };
    if( verbose_mode )
      args[ 3 ] = "-v";
    if( rfs_init( verbose_mode ? 4 : 3, args ) != 0 )      
      return 1;
  }

  log_msg( "Starting service multiplexer on %u port(s)\n", vport_num );
/*  
  // Main service thread
  while( 1 )
  {
    FD_ZERO( &fds );
    FD_SET( transport_hnd, &fds );
    for( i = 0; i < vport_num; i ++ )
      FD_SET( ports[ i ].fd, &fds );
    if( select( maxfd + 1, &fds, NULL, NULL, NULL ) <= 0 )
      continue;
    if( FD_ISSET( transport_hnd, &fds ) )
    {
      c = transport_read_byte();
      // Read one byte, interpret it
      if( c != ESCAPE_CHAR )
      {
        if( c >= SERVICE_ID_FIRST && c <= SERVICE_ID_LAST )
          service_id_in = c;
        else
        {
          if( got_esc )
          {
            // Got an escape last time, check the char now (with the 5th bit flipped)
            c ^= ESCAPE_XOR_MASK;
            if( c != ESCAPE_CHAR && c < SERVICE_ID_FIRST && c > SERVICE_ID_LAST )
            {
               log_err( "Protocol error: invalid escape sequence\n" );
               return 1;
            }
            got_esc = 0;
          }  
          if( service_id_in == -1 )
          {
            log_err( "Protocol error: service ID not specified\n" );
            return 1;
          }
          ser_write_byte( ports[ service_id_in - SERVICE_ID_FIRST ].fd, c );
        }
      }
      else
        got_esc = 1;
    }
    else
    {
      // No byte to read, check if there's something to send
      // Favour the service that sent data previously (if any)
      temp = service_id_out != -1 ? service_id_out : SERVICE_ID_FIRST;
      if( FD_ISSET( ports[ temp - SERVICE_ID_FIRST ].fd, &fds ) )
        sdata = ser_read_byte( ports[ temp - SERVICE_ID_FIRST ].fd, SER_NO_TIMEOUT );
      else
      {
        temp = -1;
        for( i = SERVICE_ID_FIRST; i < SERVICE_ID_FIRST + vport_num; i ++ )
          if( FD_ISSET( ports[ i - SERVICE_ID_FIRST ].fd, &fds ) )
          {
            temp = ( int )i;
            sdata = ser_read_byte( ports[ i - SERVICE_ID_FIRST ].fd, SER_NO_TIMEOUT );
            break;
          }
      }
      if( temp != -1 )
      {
        // We have something to send
        // Send the service ID first if needed
        if( temp != service_id_out )
          transport_send_byte( temp );
        // Then send the actual data byte, escaping it if needed
        if( sdata == ESCAPE_CHAR || ( sdata >= SERVICE_ID_FIRST && sdata <= SERVICE_ID_LAST ) )
        {
          transport_send_byte( ESCAPE_CHAR );
          transport_send_byte( ( u8 )sdata ^ ESCAPE_XOR_MASK );
        }
        else
          transport_send_byte( sdata );
        service_id_out = temp;
      }
    }
  }
*/
  return 0;
}

