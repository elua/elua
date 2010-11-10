// Remote FS server

#include "pnet.h"
#include "remotefs.h"
#include "eluarpc.h"
#include "serial.h"
#include "server.h"
#include "type.h"
#include "log.h"
#include "os_io.h"
#include <stdio.h>
#include <limits.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

// ****************************************************************************
// Local definitions

typedef void ( *p_read_request )( void );
typedef void ( *p_send_response )( void );
typedef void ( *p_cleanup )( void );
typedef struct
{
  p_read_request f_read_request;
  p_send_response f_send_response;
  p_cleanup f_cleanup;
} RFS_TRANSPORT_DATA;

// ****************************************************************************
// Local variables

#define   MAX_PACKET_SIZE     4096
static u8 rfs_buffer[ MAX_PACKET_SIZE + ELUARPC_WRITE_REQUEST_EXTRA ];
static const RFS_TRANSPORT_DATA *p_transport_data; 

// ****************************************************************************
// Serial transport implementation

static ser_handler ser;

static void flush_serial()
{
  // Flush all data in serial port
  while( ser_read_byte( ser, SER_NO_TIMEOUT ) != -1 );
}

// Read a packet from the serial port
static void ser_read_request_packet()
{
  u16 temp16;
  u32 readbytes;

  while( 1 )
  {
    // First read the length
    if( ( readbytes = ser_read( ser, rfs_buffer, ELUARPC_START_OFFSET, SER_INF_TIMEOUT ) ) != ELUARPC_START_OFFSET )
    {
      log_msg( "read_request_packet: ERROR reading packet length. Requested %d bytes, got %d bytes\n", ELUARPC_START_OFFSET, readbytes );
      flush_serial();
      continue;
    }

    if( eluarpc_get_packet_size( rfs_buffer, &temp16 ) == ELUARPC_ERR )
    {
      log_msg( "read_request_packet: ERROR getting packet size.\n" );
      flush_serial();
      continue;
    }

    // Then the rest of the data
    if( ( readbytes = ser_read( ser, rfs_buffer + ELUARPC_START_OFFSET, temp16 - ELUARPC_START_OFFSET, SER_INF_TIMEOUT ) ) != temp16 - ELUARPC_START_OFFSET )
    {
      log_msg( "read_request_packet: ERROR reading full packet, got %u bytes, expected %u bytes\n", ( unsigned )readbytes, ( unsigned )temp16 - ELUARPC_START_OFFSET );
      flush_serial();
      continue;
    }
    else
      break;
  }
}

// Send a packet to the serial port
static void ser_send_response_packet()
{
  u16 temp16;

  // Send request
  if( eluarpc_get_packet_size( rfs_buffer, &temp16 ) != ELUARPC_ERR )
  {
    log_msg( "send_response_packet: sending response packet of %u bytes\n", ( unsigned )temp16 );
    ser_write( ser, rfs_buffer, temp16 );
  }
}

static int ser_server_init( const char *portname, int serspeed )
{
  // Setup serial port
  if( ( ser = ser_open( portname ) ) == ( ser_handler )-1 )
  {
    log_err( "Cannot open port %s\n", portname );
    return 0;
  }
  if( ser_setup( ser, ( u32 )serspeed, SER_DATABITS_8, SER_PARITY_NONE, SER_STOPBITS_1 ) != SER_OK )
  {
    log_err( "Unable to initialize serial port\n" );
    return 0;
  }
  flush_serial();
  
  // User report
  log_msg( "Running RFS server on serial port %s (%u baud).\n", portname, ( unsigned )serspeed );
  return 1; 
}

static void ser_cleanup()
{
  ser_close( ser );
}

static const RFS_TRANSPORT_DATA ser_transport_data = { ser_read_request_packet, ser_send_response_packet, ser_cleanup };

// ****************************************************************************
// UDP transport implementation

static SOCKET trans_socket = INVALID_SOCKET_VALUE;
static struct sockaddr_in trans_from;

// Helper: read (blocking) the specified number of bytes

static void udp_read_helper( u8 *dest, u32 size )
{
  socklen_t fromlen;
  int readbytes;

  while( size )
  {
    fromlen = sizeof( trans_from );
    readbytes = recvfrom( trans_socket, ( char* )dest, size, 0, ( struct sockaddr* )&trans_from, &fromlen );
    size -= readbytes;
    if( size == 0 )
      break;
    dest += readbytes;
  }
}

static void udp_read_request_packet()
{
  u16 temp16;
 
  while( 1 )
  {
    // First read the length
    udp_read_helper( rfs_buffer, ELUARPC_START_OFFSET );

    if( eluarpc_get_packet_size( rfs_buffer, &temp16 ) == ELUARPC_ERR )
    {
      log_msg( "read_request_packet: ERROR getting packet size.\n" );
      continue;
    }

    // Then the rest of the data
    udp_read_helper( rfs_buffer + ELUARPC_START_OFFSET, temp16 - ELUARPC_START_OFFSET );
    break;
  }
}

static void udp_send_response_packet()
{
  u16 temp16;
  
  // Send request
  if( eluarpc_get_packet_size( rfs_buffer, &temp16 ) != ELUARPC_ERR )
  {
    log_msg( "send_response_packet: sending response packet of %u bytes\n", ( unsigned )temp16 );
    sendto( trans_socket, ( char* )rfs_buffer, temp16, 0, ( struct sockaddr* )&trans_from, sizeof( trans_from ) );
  }  
}

static int udp_server_init( unsigned server_port )
{
  int length;
  struct sockaddr_in server;
     
  if( ( trans_socket = socket( AF_INET, SOCK_DGRAM, 0 ) ) < 0 )
  {
    log_err( "Unable to create socket\n" );
    return 1;
  }
  length = sizeof( server );
  memset( &server, 0, sizeof( server ) );
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons( server_port );
  if( bind( trans_socket, ( struct sockaddr * )&server, length ) < 0 )
  {
   log_err( "Unable to bind socket\n" );
   return 0; 
  }
  log_msg( "Running RFS server on UDP port %u.\n", ( unsigned )server_port );
  return 1;    
}

static void udp_cleanup()
{
}

static const RFS_TRANSPORT_DATA udp_transport_data = { udp_read_request_packet, udp_send_response_packet, udp_cleanup };

// *****************************************************************************
// Helper functions

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
static int parse_transport_and_init( const char* s )
{
  const char *c;
  char *temps;
  long tempi;
  
  if( strstr( s, "ser:" ) == s )
  {
    p_transport_data = &ser_transport_data;
    s += strlen( "ser:" );
    if( ( c = strchr( s, ',' ) ) == NULL )
    {
      log_err( "Invalid serial transport syntax\n" );
      return 0;
    }
    if( secure_atoi( c + 1, &tempi ) == 0 )
    {
      log_err( "Invalid port speed\n" );
      return 0;
    }
    temps = l_strndup( s, c - s );
    tempi = ser_server_init( temps, tempi );
    free( temps );    
    return tempi;
  }
  else if( strstr( s, "udp:" ) == s )
  {
    p_transport_data = &udp_transport_data;
    s += strlen( "udp:" );
    if( secure_atoi( s, &tempi ) == 0 )
    {
      log_err( "Invalid port number\n" );
      return 0;
    }    
    return udp_server_init( tempi );   
  }
  log_err( "Error: unsupported transport\n" );
  return 0;
}

// *****************************************************************************
// Entry point

#ifdef RFS_STANDALONE_MODE
#define RFS_MAIN_FUNC_NAME    main
#else
#define RFS_MAIN_FUNC_NAME    rfs_main
#endif

#define TRANSPORT_ARG_IDX     1
#define DIRNAME_ARG_IDX       2
#define VERBOSE_ARG_IDX       3
#define MIN_ARGC_COUNT        3
#define VERBOSE_ARGC_COUNT    4

int RFS_MAIN_FUNC_NAME( int argc, const char **argv )
{  
  if( argc < MIN_ARGC_COUNT )
  {
    log_err( "Usage: %s <transport> <dirname> [-v]\n", argv[ 0 ] );
    log_err( "  Serial transport: 'ser:<sername>,<serspeed>'" );
    log_err( "  UDP transport: 'udp:<port>'" );
    log_err( "Use -v for verbose output.\n");
    return 1;
  }
  if( !os_isdir( argv[ DIRNAME_ARG_IDX ] ) )
  {
    log_err( "Invalid directory %s\n", argv[ DIRNAME_ARG_IDX ] );
    return 1;
  }  
  if( !parse_transport_and_init( argv[ TRANSPORT_ARG_IDX ] ) == 0 )
    return 1;
  
    // Setup RFS server
  server_setup( argv[ DIRNAME_ARG_IDX ] );   
  log_msg( "Running RFS server on directory %s\n", argv[ DIRNAME_ARG_IDX ] );

  if( ( argc >= VERBOSE_ARGC_COUNT ) && !strcmp( argv[ VERBOSE_ARG_IDX ], "-v" ) )
    log_init( LOG_ALL );
  else
    log_init( LOG_NONE );

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


