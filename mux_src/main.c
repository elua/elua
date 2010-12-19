// Service multiplexer 

#include "net.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>
#include "config.h"
#include "log.h"
#include "type.h"
#include "serial.h"
#include "sermux.h"
#include "rfs.h"
#include "deskutils.h"

// ****************************************************************************
// Data structures and local variables

#define MODE_MUX              1
#define MODE_RFSMUX           2

#define HND_TRANSPORT_OFFSET  0
#define HND_FIRST_VOFFSET     1

#define RFS_PSEUDO_SELIDX     0xFF

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
  int flow;
} TRANSPORT_SER;

static SERVICE_DATA *services;
static unsigned vport_num;

static TRANSPORT_SER *transport_data;
static p_send_func transport_send;
static p_init_func transport_init;

static int service_id_in = -1, service_id_out = -1;
 
static ser_handler transport_hnd = SER_HANDLER_INVALID;
static int mux_mode;
static int verbose_mode;
static int rfs_service_id = -1, service_offset;

// ***************************************************************************
// Serial transport implementation

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
  if( ser_setup( pser->fd, pser->speed, 8, SER_PARITY_NONE, SER_STOPBITS_1, pser->flow ) != SER_OK )
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

// Transport parser
static int parse_transport( const char* s )
{
  const char *c, *c2;
  static TRANSPORT_SER tser;
  char *temp;

  if( ( c = strchr( s, ',' ) ) == NULL )
  {
    log_err( "Invalid serial transport syntax.\n" );
    return 0;
  }
  tser.pname = l_strndup( s, c - s );
  if( ( c2 = strchr( c + 1, ',' ) ) == NULL )
  {
    log_err( "Invalid serial transport syntax.\n" );
    return 0;
  }
  temp = l_strndup( c + 1, c2 - c - 1 );
  if( secure_atoi( temp, &tser.speed ) == 0 )
  {
    log_err( "Invalid port speed\n" );
    return 0;
  }
  free( temp );
  if( !strcmp( c2 + 1, "none" ) )
    tser.flow = SER_FLOW_NONE;
  else if( !strcmp( c2 + 1, "rtscts" ) )
    tser.flow = SER_FLOW_RTSCTS;
  else
  {
    log_err( "Invalid flow control type.\n" );
    return 0;
  }
  transport_data = &tser;
  transport_send = transport_ser_send;
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
  int c, prev_sent = -1;
  int temp;
  int got_esc = 0;
  char* rfs_dir_name;
  ser_handler *phandlers;
  int selidx;
  u16 rfs_size = 0;
  u8 *rfs_ptr;

  // Interpret arguments
  setvbuf( stdout, NULL, _IONBF, 0 );  
  if( argc < MIN_ARGC_COUNT )
  {
    log_err( "Usage: %s <mode> <transport> <vcom1> [<vcom2>] ... [<vcomn>] [-v]\n", argv[ 0 ] );
    log_err( "  mode: \n" );
    log_err( "    'mux':                 serial multiplexer mode\n" );
    log_err( "    'rfsmux:<directory>:   combined RFS and multiplexer mode.\n" );
    log_err( "  transport: '<port>,<baud>,<flow> ('flow' specifies the flow control type and can be 'none' or 'rtscts').\n" );
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
    rfs_service_id = SERMUX_SERVICE_ID_FIRST;
    service_offset = 1;
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
  if( ( phandlers = ( ser_handler* )malloc( sizeof( ser_handler ) * ( vport_num + 1 ) ) ) == NULL )
  {
    log_err( "Not enough memory\n" );
    return 1;  
  }
  phandlers[ HND_TRANSPORT_OFFSET ] = transport_hnd;

  memset( services, 0, sizeof( SERVICE_DATA ) * vport_num );
  for( i = 0; i < vport_num; i ++ ) 
  {
    tservice = services + i;
    if( ( tservice->fd = ser_open( argv[ i + FIRST_SERVICE_IDX ] ) ) == SER_HANDLER_INVALID )
    {
      log_err( "Unable to open port %s\n", argv[ i + FIRST_SERVICE_IDX ] );
      return 1;
    }
    if( ser_setup( tservice->fd, transport_data->speed, SER_DATABITS_8, SER_PARITY_NONE, SER_STOPBITS_1, SER_FLOW_NONE ) != SER_OK )
    {
      log_err( "Unable to setup serial port %s\n", argv[ i + FIRST_SERVICE_IDX ] );
      return 1;
    }
    tservice->pname = argv[ i + FIRST_SERVICE_IDX ];
    phandlers[ i + HND_FIRST_VOFFSET ] = tservice->fd;
  }
  
  // Setup RFS server in RFSMUX mode
  if( mux_mode == MODE_RFSMUX )
  {
    char *args[] = { "dummy", "mem", rfs_dir_name, NULL };
    if( verbose_mode )
      args[ 3 ] = "-v";
    if( rfs_init( verbose_mode ? 4 : 3, ( const char ** )args ) != 0 )      
      return 1;
  }

  log_msg( "Starting service multiplexer on %u port(s)\n", vport_num );
  
  // Main service thread
  while( 1 )
  {
    if( rfs_size > 0 ) // Response packet from RFS
    {
      c = *rfs_ptr ++;
      rfs_size --;
      selidx = RFS_PSEUDO_SELIDX;
    }
    else
    {
      if( ( c = ser_select_byte( phandlers, vport_num + 1, SER_INF_TIMEOUT ) ) == -1 )
      {
        log_err( "Error on select, aborting program\n" );
        return 1;
      }
      selidx = c >> 8;
      c = c & 0xFF;
    }
    //log_msg( "Got byte %d from idx %d\n", c, selidx );
    if( selidx == HND_TRANSPORT_OFFSET ) // Got byte on transport interface
    {
      // Interpret byte
      if( c != SERMUX_ESCAPE_CHAR )
      {
        if( c >= SERMUX_SERVICE_ID_FIRST && c <= SERMUX_SERVICE_ID_LAST )
        {
          log_msg( "Changed service_id_in from %d(%X) to %d(%X).\n", service_id_in, service_id_in, c, c );
          service_id_in = c;
        } 
        else if( c == SERMUX_FORCE_SID_CHAR )
        {
          if( prev_sent == -1 )
          {
            log_err( "Protocol error: got request to resend service ID when the last char sent was not set.\n" );
            return 1;
          }
          log_msg( "Got request to resend service_id_out %d(%X).\n", service_id_out, service_id_out );
          // Re-transmit the last data AND the service ID
          transport_send_byte( service_id_out );
          if( prev_sent & SERMUX_ESC_MASK )
            transport_send_byte( SERMUX_ESCAPE_CHAR );
          transport_send_byte( prev_sent & 0xFF );
          prev_sent = -1;
        }          
        else
        {
          if( got_esc )
          {
            // Got an escape last time, check the char now (with the 5th bit flipped)
            c ^= SERMUX_ESCAPE_XOR_MASK;
            if( c != SERMUX_ESCAPE_CHAR && c != SERMUX_FORCE_SID_CHAR && ( c < SERMUX_SERVICE_ID_FIRST || c > SERMUX_SERVICE_ID_LAST ) )
            {
               log_err( "Protocol error: invalid escape sequence\n" );
               return 1;
            }
            got_esc = 0;
          }  
          if( service_id_in == -1 )
          {
            transport_send_byte( SERMUX_FORCE_SID_CHAR );
            log_msg( "Requested resend of service ID for byte %3d ('%c').\n", c, isprint( c ) ? c : ' ' );
          }
          else
          {
            if( service_id_in == rfs_service_id ) // this request is for the RFS server
            {
              rfs_mem_read_request_packet( c );
              if( rfs_mem_has_response() ) // we have a response from the RFS server
              {
                rfs_mem_write_response( &rfs_size, &rfs_ptr );                  
                rfs_mem_start_request(); // initialize the RFS server for a new request  
              }
            }
            else
            {
              //log_msg( "Sending byte %d to %s\n", c, services[ service_id_in - SERMUX_SERVICE_ID_FIRST - service_offset ].pname );
              ser_write_byte( services[ service_id_in - SERMUX_SERVICE_ID_FIRST - service_offset ].fd, c );
            }
          }
        }
      }
      else
        got_esc = 1;                          
    }
    else
    {
      // No byte to read, there must be something to send
      if( selidx == RFS_PSEUDO_SELIDX )
        temp = SERMUX_SERVICE_ID_FIRST;
      else        
        temp = SERMUX_SERVICE_ID_FIRST + selidx - HND_FIRST_VOFFSET + service_offset;
      prev_sent = c;
      // Send the service ID first if needed
      if( temp != service_id_out )
      {
        log_msg( "Changed service_id_out from %d(%X) to %d(%X).\n", service_id_out, service_id_out, temp, temp );
        transport_send_byte( temp );
      }
      // Then send the actual data byte, escaping it if needed
      if( c == SERMUX_ESCAPE_CHAR || c == SERMUX_FORCE_SID_CHAR || ( c >= SERMUX_SERVICE_ID_FIRST && c <= SERMUX_SERVICE_ID_LAST ) )
      {
        transport_send_byte( SERMUX_ESCAPE_CHAR );
        transport_send_byte( ( u8 )c ^ SERMUX_ESCAPE_XOR_MASK );
        prev_sent = SERMUX_ESC_MASK | ( ( u8 )c ^ SERMUX_ESCAPE_XOR_MASK );
      }
      else
        transport_send_byte( c );
      service_id_out = temp;
    }
  }

  return 0;
}

