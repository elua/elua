// Service multiplexer 

#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include "config.h"
#include "log.h"
#include "type.h"
#include "serial.h"

// ****************************************************************************
// Data structures and local variables

#define TRANSPORT_TYPE_ERROR        0
#define TRANSPORT_TYPE_SER          1

// Service ID of the transport thread
#define TRANSPORT_SERVICE_ID        -1

// Send/receive/init function pointers
typedef u32 ( *p_recv_func )( u8 *p, u32 size );
typedef u32 ( *p_send_func )( const u8 *p, u32 size );
typedef int ( *p_init_func )( void );

// Serial thread buffer structure
typedef struct 
{
  pthread_t tid;
  const char *pname;
  ser_handler fd;
  int service_id;
} THREAD_DATA;

// Serial transport data structure
typedef struct 
{
  ser_handler fd;
  const char *pname;
  long speed;
} TRANSPORT_SER;

// Service/transport data
typedef struct 
{
  int id;
  u8 data;
} DATA;

static THREAD_DATA *threads;
static unsigned vport_num;

static void *transport_data;
static int transport_type;
static p_recv_func transport_recv;
static p_send_func transport_send;
static p_init_func transport_init;

static long service_baud;
static sem_t mux_w_sem, mux_r_sem;
static DATA mux_data;
static int service_id_in = -1, service_id_out = -1;

// ***************************************************************************
// Serial transport implementation

static u32 transport_ser_recv( u8 *p, u32 size ) 
{
  TRANSPORT_SER *pser = ( TRANSPORT_SER* )transport_data;

  return ser_read( pser->fd, p, size, SER_INF_TIMEOUT );
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
    fprintf( stderr, "Unable to open %s\n", pser->pname );
    return 0;
  }
  if( ser_setup( pser->fd, pser->speed, 8, SER_PARITY_NONE, SER_STOPBITS_1 ) != SER_OK )
  {
    fprintf( stderr, "Unable to setup serial port %s\n", pser->pname );
    return 0;
  }
  while( ser_read_byte( pser->fd, SER_NO_TIMEOUT ) != -1 );
  printf( "Running serial transport on port %s at %u baud (8N1)\n", pser->pname, ( unsigned )pser->speed );
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

// Transport parser
static int parse_transport( const char* s )
{
  const char *c;
  static TRANSPORT_SER tser; 

  if( strstr( s, "ser:" ) == s )
  {
    s += strlen( "ser:" );
    if( ( c = strchr( s, ',' ) ) == NULL )
    {
      fprintf( stderr, "Invalid serial transport syntax\n" );
      return 0;
    }
    if( secure_atoi( c + 1, &tser.speed ) == 0 )
    {
      fprintf( stderr, "Invalid port speed\n" );
      return 0;
    }
    tser.pname = strndup( s, c - s );
    transport_data = &tser;
    transport_send = transport_ser_send;
    transport_recv = transport_ser_recv;
    transport_init = transport_ser_init;
    transport_type = TRANSPORT_TYPE_SER;
    return 1;
  }
  fprintf( stderr, "Error: unsupported transport\n" );
  return 0;
}

// ***************************************************************************
// Service thread

static void* service_thread( void* data )
{
  THREAD_DATA *pdata = ( THREAD_DATA*) data;
  int sdata;

  log_msg( "Starting service thread for port %s\n", pdata->pname );
  while( 1 ) 
  {
    if( ( sdata = ser_read_byte( pdata->fd, SER_INF_TIMEOUT ) ) == -1 )
      continue;
    sem_wait( &mux_w_sem );
    mux_data.data = sdata;
    mux_data.id = pdata->service_id;
    sem_post( &mux_r_sem );
  }
  return NULL;
}

// ****************************************************************************
// Transport thread

static void* transport_thread( void* data )
{
  int sdata;

  log_msg( "Starting transport thread\n" );
  while( 1 ) 
  {
    if( ( sdata = transport_read_byte() ) == -1 )
      continue;
    sem_wait( &mux_w_sem );
    mux_data.data = sdata;
    mux_data.id = TRANSPORT_SERVICE_ID;
    sem_post( &mux_r_sem );
  }
  return NULL;
}

// ****************************************************************************
// Program entry point

#define MAIN_TRANSPORT_IDX    1
#define SERVICE_BAUD_IDX      2
#define FIRST_SERVICE_IDX     3

int main( int argc, char **argv )
{
  unsigned i;
  THREAD_DATA *tdata;
  int c;
  int temp, sdata;
  int got_esc = 0;
  DATA data;

  // Interpret arguments
  if( argc < 4 )
  {
    fprintf( stderr, "Usage: %s <transport> <baud> <vcom1> [<vcom2>] ... [<vcomn>] [-v]\n", argv[ 0 ] );
    return 1;
  }
  i = argc - 1;
  if( !strcasecmp( argv[ i ], "-v" ) )
  {
    i --;
    log_init( LOG_ALL );
  }
  else
    log_init( LOG_NONE ); 
  if( ( vport_num = i - 2 ) > SERVICE_MAX )
  {
    fprintf( stderr, "Too many service ports, maximum is %d\n", SERVICE_MAX );
    return 1;
  }
  if( parse_transport( argv[ MAIN_TRANSPORT_IDX ] ) == 0 )
    return 1;
  if( secure_atoi( argv[ SERVICE_BAUD_IDX ], &service_baud ) == 0 )
  {
    fprintf( stderr, "Invalid service baud\n" );
    return 1;
  }
  if( transport_init() == 0 )
    return 1;

  // Create global sync objects
  sem_init( &mux_w_sem, 0, 1 );
  sem_init( &mux_r_sem, 0, 0 );

  // Open all the service ports and create their corresponding threads
  if( ( threads = ( THREAD_DATA* )malloc( sizeof( THREAD_DATA ) * ( vport_num + 1 ) ) ) == NULL )
  {
    fprintf( stderr, "Not enough memory\n" );
    return 1;
  }
  for( i = 0; i <= vport_num; i ++ ) 
  {
    tdata = threads + i;
    if( i < vport_num )
    {
      if( ( tdata->fd = ser_open( argv[ i + FIRST_SERVICE_IDX ] ) ) == ( ser_handler )-1 )
      {
        fprintf( stderr, "Unable to open port %s\n", argv[ i + FIRST_SERVICE_IDX ] );
        return 1;
      }
      if( ser_setup( tdata->fd, service_baud, SER_DATABITS_8, SER_PARITY_NONE, SER_STOPBITS_1 ) != SER_OK )
      {
        fprintf( stderr, "Unable to setup serial port %s\n", argv[ i + FIRST_SERVICE_IDX ] );
        return 1;
      }
      tdata->pname = argv[ i + FIRST_SERVICE_IDX ];
      tdata->service_id = i + SERVICE_ID_FIRST;
    }
    if( pthread_create( &tdata->tid, NULL, i == vport_num ? transport_thread : service_thread, ( void* )tdata ) )
    {
      fprintf( stderr, "Unable to create thread\n" );
      return 1;
    }
  }

  printf( "Starting service multiplexer on %u port(s)\n", vport_num );
  // Main service thread
  while( 1 )
  {
    sem_wait( &mux_r_sem );
    data = mux_data;
    sem_post( &mux_w_sem );
    if( data.id == TRANSPORT_SERVICE_ID )
    {
      // Read one byte, interpret it
      c = data.data;
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
               fprintf( stderr, "Protocol error: invalid escape sequence\n" );
               return 1;
            }
            got_esc = 0;
          }  
          if( service_id_in == -1 )
          {
            fprintf( stderr, "Protocol error: service ID not specified\n" );
            return 1;
          }
          ser_write_byte( threads[ service_id_in - SERVICE_ID_FIRST ].fd, c );
        }
      }
      else
        got_esc = 1;
    }
    else
    {
      temp = data.id;
      sdata = data.data;
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

  return 0;
}

