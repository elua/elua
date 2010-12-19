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
#include "os_io.h"
#include "pnet.h"

// External RFS functions
extern int rfs_server_init( unsigned server_port, const char* dirname );
extern void rfs_server_cleanup();
extern void* rfs_thread( void* data );
extern volatile int rfs_thread_should_die;

// ****************************************************************************
// Data structures and local variables

#define TRANSPORT_TYPE_ERROR        0
#define TRANSPORT_TYPE_SER          1
#define TRANSPORT_TYPE_UDP          2
#define TRANSPORT_TYPE_TEXTCTRL     3

// Service ID of the transport thread
#define TRANSPORT_SERVICE_ID        ( -1 )
#define THREAD_STOP_SERVICE_ID      ( -2 )  

// Thread indexes in the threads array
#ifdef MUX_THREAD_MODE
#define TEXTCTRL_THREAD_IDX         1
#else
#define TEXTCTRL_THREAD_IDX         ( -1 )
#endif
#define RFS_UDP_THREAD_IDX          0
        
// Send/receive/init function pointers for transport
typedef u32 ( *p_recv_func )( u8 *p, u32 size );
typedef u32 ( *p_send_func )( const u8 *p, u32 size );
typedef int ( *p_init_func )( void );
typedef void ( *p_cleanup_func )( void );

// Thread function type
typedef void* ( *p_thread_func )( void* );

// Thread structure
typedef struct 
{
  pthread_t tid;
  int thread_created;
  volatile int thread_should_die;
  int type;  
  union
  {
    const char *pname;
    unsigned port;
  };
  union
  {
    ser_handler fd;
    SOCKET s;
  };
  int service_id;
  struct sockaddr_in server;
  union
  {
    sem_t udp_sem;
    sem_t textctrl_sem;
  };
  int textctrl_data;
  u8 *udp_buf;
} THREAD_DATA;

// Serial transport data structure
typedef struct 
{
  ser_handler fd;
  char *pname;
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
static p_cleanup_func transport_cleanup;
static long service_baud;
static sem_t mux_w_sem, mux_r_sem;
static DATA mux_data;
static int service_id_in = -1, service_id_out = -1;
static pthread_t rfs_thread_id;
static int rfs_thread_created;
volatile int mux_thread_running;

// *****************************************************************************
// wxTextCtrl "transport" helpers

#ifdef MUX_THREAD_MODE

// wxTextCtrl buffer data
typedef struct
{
  int *pdata;
  unsigned r_idx, w_idx, total;  
  sem_t count_sem;
} TEXTCTRL_DATA;

static TEXTCTRL_DATA textctrl_data;

int mux_textctrl_init( unsigned total )
{                                  
  if( ( textctrl_data.pdata = ( int* )malloc( total * sizeof( int ) ) ) == NULL )
  {
    log_err( "Unable to alloc textctrl buffer\n" );
    return 0;
  }
  textctrl_data.r_idx = textctrl_data.w_idx = 0;
  textctrl_data.total = total;
  sem_init( &textctrl_data.count_sem, 0, 0 );
  return 1;
}

void mux_textctrl_cleanup()
{
  free( textctrl_data.pdata );
  sem_destroy( &textctrl_data.count_sem );
}

void textctrl_get_char( int c )
{
#if 0
  textctrl_data.pdata[ textctrl_data.w_idx ] = c;
  textctrl_data.w_idx = ( textctrl_data.w_idx + 1 ) % textctrl_data.total;
  printf( "!" );
  sem_post( &textctrl_data.count_sem );
#else
  THREAD_DATA *pdata = threads + TEXTCTRL_THREAD_IDX;
  pdata->textctrl_data = c;
  sem_post( &pdata->textctrl_sem );
#endif
}

extern void textctrl_put_char( int c );

#else // #ifdef MUX_THREAD_MODE

static void textctrl_put_char( int c )
{
}

static void textctrl_get_char( int c )
{
}

#endif // #ifdef MUX_THREAD_MODE

// *****************************************************************************
// Serial transport implementation

static u32 transport_ser_recv( u8 *p, u32 size ) 
{
  TRANSPORT_SER *pser = ( TRANSPORT_SER* )transport_data;

  return ser_read( pser->fd, p, size, SER_TIMEOUT_MS );
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
  while( ser_read_byte( pser->fd, SER_NO_TIMEOUT ) != -1 );
  log_msg( "Running serial transport on port %s at %u baud (8N1)\n", pser->pname, ( unsigned )pser->speed );
  return 1;
}

static void transport_ser_cleanup()
{
  TRANSPORT_SER *pser = ( TRANSPORT_SER* )transport_data;

  if( pser->fd && pser->fd != ( ser_handler )-1 )
    ser_close( pser->fd );
  free( pser->pname );
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

  if( strstr( s, "ser:" ) == s )
  {
    s += strlen( "ser:" );
    if( ( c = strchr( s, ',' ) ) == NULL )
    {
      log_err( "Invalid serial transport syntax\n" );
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
    transport_cleanup = transport_ser_cleanup;
    transport_type = TRANSPORT_TYPE_SER;
    return 1;
  }
  log_err( "Error: unsupported transport\n" );
  return 0;
}

// ***************************************************************************
// Service threads

static void* service_thread_ser( void* data )
{
  THREAD_DATA *pdata = ( THREAD_DATA* )data;
  int sdata;

  log_msg( "Starting serial service thread for port %s\n", pdata->pname );
  while( 1 ) 
  {
    if( ( sdata = ser_read_byte( pdata->fd, SER_TIMEOUT_MS ) ) == -1 )
    {
      if( pdata->thread_should_die )
        break;
      else
        continue;
    }
    sem_wait( &mux_w_sem );
    mux_data.data = sdata;
    mux_data.id = pdata->service_id;
    sem_post( &mux_r_sem );
  }
  return NULL;
}

static void* service_thread_udp( void* data )
{
  THREAD_DATA *pdata = ( THREAD_DATA* )data;
  socklen_t fromlen;
  struct sockaddr_in from;
  int readbytes, i;
  fd_set fds;
  struct timeval tv;
    
  log_msg( "Starting UDP service thread for port %d\n", pdata->port );
  while( 1 ) 
  {
    FD_ZERO( &fds );
    FD_SET( pdata->s, &fds );
    tv.tv_sec = 0;
    tv.tv_usec = NET_TIMEOUT_MS * 1000;
    if( select( pdata->s + 1, &fds, NULL, NULL, &tv ) <= 0 )
    {
      if( pdata->thread_should_die )
        break;
      else
        continue;
    }
    fromlen = sizeof( from );
    readbytes = recvfrom( pdata->s, pdata->udp_buf, UDP_BUF_SIZE, 0, ( struct sockaddr* )&from, &fromlen );
    for( i = 0; i < readbytes; i ++ )
    {
      if( pdata->thread_should_die )
        break;    
      sem_wait( &mux_w_sem );
      mux_data.data = pdata->udp_buf[ i ];
      mux_data.id = pdata->service_id;
      sem_post( &mux_r_sem );
    }
    if( pdata->thread_should_die )
      break;       
  }
  return NULL;  
}

static void* service_thread_textctrl( void* data )
{
#ifdef MUX_THREAD_MODE
  int c;
  THREAD_DATA *pdata = ( THREAD_DATA* )data;  
    
  log_msg( "Starting wxTextCtrl service thread\n" );
  while( 1 )
  {
#if 0  
    sem_wait( &textctrl_data.count_sem );
    if( ( c = textctrl_data.pdata[ textctrl_data.r_idx ] ) == -1 )
      break;
    textctrl_data.r_idx = ( textctrl_data.r_idx + 1 ) % textctrl_data.total;
#else
    sem_wait( &pdata->textctrl_sem );
    if( ( c = pdata->textctrl_data ) == -1 )
      break;
#endif
    sem_wait( &mux_w_sem );
    mux_data.data = c;
    mux_data.id = pdata->service_id;
    sem_post( &mux_r_sem );      
  }
#endif  
  return NULL;
}

static const p_thread_func mux_service_funcs[] = { NULL, service_thread_ser, service_thread_udp, service_thread_textctrl };

// *****************************************************************************
// Transport thread

static void* transport_thread( void* data )
{                     
  int sdata;
  THREAD_DATA *pdata = ( THREAD_DATA* )data;
  
  log_msg( "Starting transport thread\n" );  
  while( 1 ) 
  {
    if( ( sdata = transport_read_byte() ) == -1 )
    {    
      if( pdata->thread_should_die )
        break;
      else
        continue;
    }    
    sem_wait( &mux_w_sem );
    mux_data.data = sdata;
    mux_data.id = TRANSPORT_SERVICE_ID;
    sem_post( &mux_r_sem );
  }
  return NULL;
}

// *****************************************************************************
// Transport aware byte send function

static void byte_send( THREAD_DATA *data, u8 d )
{   
  if( data->type == TRANSPORT_TYPE_SER )
    ser_write_byte( data->fd, d );
  else if( data->type == TRANSPORT_TYPE_UDP )
    sendto( data->s, &d, 1, 0, ( struct sockaddr* )&data->server, sizeof( data->server ) );
  else 
    textctrl_put_char( d );            
}

// ****************************************************************************
// Program entry point

#define MAIN_TRANSPORT_IDX    1
#define SERVICE_BAUD_IDX      2
#define RFS_DIRNAME_IDX       3
#define FIRST_SERVICE_IDX     4

int mux_init( int argc, char **argv )
{
  unsigned i;
  THREAD_DATA *tdata;
  struct hostent *hp;
        
  // Setup networking in Windows
#ifdef WIN32_BUILD
  // The socket subsystem must be initialized if working in Windows
  WORD wVersionRequested;
  WSADATA wsaData;
  int err;
 
  wVersionRequested = MAKEWORD( 2, 0 );
  err = WSAStartup( wVersionRequested, &wsaData );  
  if( err != 0 )
  {
    log_err( "Unable to initialize the socket subsystem\n" );
    return 1;
  }
#endif // #ifdef WIN32_BUILD  

  // Interpret arguments
  if( argc < FIRST_SERVICE_IDX + 1 )
  {
    log_err( "Usage: %s <transport> <baud> <rfs_dir_name> <vcom1> [<vcom2>] ... [<vcomn>] [-v]\n", argv[ 0 ] );
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
  if( ( vport_num = i - 3 ) > SERVICE_MAX )
  {
    log_err( "Too many service ports, maximum is %d\n", SERVICE_MAX );
    return 1;
  }
  if( parse_transport( argv[ MAIN_TRANSPORT_IDX ] ) == 0 )
    return 1;
  if( secure_atoi( argv[ SERVICE_BAUD_IDX ], &service_baud ) == 0 )
  {
    log_err( "Invalid service baud\n" );
    return 1;
  }
  if( !os_isdir( argv[ RFS_DIRNAME_IDX ] ) )
  {
    log_err( "Invalid directory %s\n", argv[ RFS_DIRNAME_IDX ] );
    return 1;
  }  
  if( transport_init() == 0 )
    return 1;
  if( rfs_server_init( RFS_SERVER_PORT, argv[ RFS_DIRNAME_IDX ] ) == 0 )
    return 1;
      
  // Create global sync objects
  sem_init( &mux_w_sem, 0, 1 );
  sem_init( &mux_r_sem, 0, 0 );

  // Open all the service ports and create their corresponding threads
  if( ( threads = ( THREAD_DATA* )malloc( sizeof( THREAD_DATA ) * ( vport_num + 2 ) ) ) == NULL )
  {
    log_err( "Not enough memory\n" );
    return 1;
  }
  memset( threads, 0, sizeof( THREAD_DATA ) * ( vport_num + 2 ) );
  // Thread map:
  // index 0 : UDP thread (ALWAYS first service ID!)
  // 1 ... vport_num : actual serial ports
  //   (1 is connected to the console wxTextCtrl in frontend mode)
  // vport_num + 1: transport thread
  for( i = 0; i <= vport_num + 1; i ++ ) 
  {
    tdata = threads + i;
    tdata->thread_should_die = 0;
    if( i <= vport_num )
    {    
      if( i == TEXTCTRL_THREAD_IDX )
      {
        sem_init( &tdata->textctrl_sem, 0, 0 );          
        tdata->type = TRANSPORT_TYPE_TEXTCTRL;
      } 
      else if( i == RFS_UDP_THREAD_IDX )    
      {      
        tdata->s = INVALID_SOCKET_VALUE;
        if( ( tdata->udp_buf = ( u8* )malloc( UDP_BUF_SIZE ) ) == NULL )
        {
          log_err( "Unable to allocate UDP buffer\n" );
          return 1;
        }
        // UDP thread  
        if( ( tdata->s = socket( AF_INET, SOCK_DGRAM, 0 ) ) == INVALID_SOCKET_VALUE )
        {
          log_err( "Error creating socket\n" );
          return 1;
        }
        tdata->server.sin_family = AF_INET;
        hp = gethostbyname( "127.0.0.1" );
        memcpy( &tdata->server.sin_addr, hp->h_addr, hp->h_length );
        tdata->server.sin_port = htons( RFS_SERVER_PORT );
        tdata->type = TRANSPORT_TYPE_UDP;
        tdata->port = RFS_SERVER_PORT;
        sem_init( &tdata->udp_sem, 0, 0 );                  
      }
      else
      {   
        // Serial thread
        if( ( tdata->fd = ser_open( argv[ i  - 1 + FIRST_SERVICE_IDX ] ) ) == ( ser_handler )-1 )
        {
          log_err( "Unable to open port %s\n", argv[ i - 1 + FIRST_SERVICE_IDX ] );
          return 1;
        }
        if( ser_setup( tdata->fd, service_baud, SER_DATABITS_8, SER_PARITY_NONE, SER_STOPBITS_1 ) != SER_OK )
        {
          log_err( "Unable to setup serial port %s\n", argv[ i - 1 + FIRST_SERVICE_IDX ] );
          return 1;
        }
        tdata->type = TRANSPORT_TYPE_SER;
        tdata->pname = argv[ i - 1 + FIRST_SERVICE_IDX ];        
      }
      tdata->service_id = i + SERVICE_ID_FIRST;         
    }
    if( pthread_create( &tdata->tid, NULL, i <= vport_num ? mux_service_funcs[ tdata->type ] : transport_thread, tdata ) )
    {
      log_err( "Unable to create transport thread\n" );
      return 1;
    }
    tdata->thread_created = 1;
  }
  // Create rfs_server thread
  if( pthread_create( &rfs_thread_id, NULL, rfs_thread, argv[ RFS_DIRNAME_IDX ] ) )
  {
    log_err( "Unable to create RFS thread\n" );
    return 1;
  }
  rfs_thread_created = 1;
  return 0;  
}

void mux_cleanup()
{
  unsigned i;
  THREAD_DATA *tdata;
                            
  if( threads )
  {
    // Thread map:
    // index 0 : UDP thread (ALWAYS first service ID!)
    // 1 ... vport_num : actual serial ports
    // vport_num + 1: transport thread
    for( i = 0; i <= vport_num + 1; i ++ ) 
    {
      tdata = threads + i;
      if( tdata->thread_created == 1 )
      {
        if( i == TEXTCTRL_THREAD_IDX )
          textctrl_get_char( -1 );
        tdata->thread_should_die = 1;
        pthread_join( tdata->tid, NULL );
      }
      if( i <= vport_num )
      {
        if( i == TEXTCTRL_THREAD_IDX )
          sem_destroy( &tdata->textctrl_sem );
        else if( i == RFS_UDP_THREAD_IDX )
        {        
          free( tdata->udp_buf );
          if( tdata->s != INVALID_SOCKET_VALUE )
            socket_close( tdata->s );             
          sem_destroy( &tdata->udp_sem );                   
        }
        else
        {  
          // Serial thread
          if( tdata->fd && tdata->fd != ( ser_handler )-1 )
            ser_close( tdata->fd );         
        }    
      }
    }
    free( threads );
    threads = NULL;
  }
  if( rfs_thread_created )
  {
    rfs_thread_should_die = 1;
    pthread_join( rfs_thread_id, NULL );
  }
  rfs_server_cleanup();
  transport_cleanup();
  
  sem_destroy( &mux_w_sem );
  sem_destroy( &mux_r_sem );
  
  // Re-init globals
  vport_num = 0;
  transport_type = TRANSPORT_TYPE_ERROR;
  service_baud = 0;
  service_id_in = -1;
  service_id_out = -1;
  rfs_thread_created = 0;
  mux_thread_running = 0;
}

#if defined( MUX_STANDALONE_MODE )

#define RETVAL  1
#define RETVAL_TYPE int

#elif defined( MUX_THREAD_MODE )

#define RETVAL  NULL
#define RETVAL_TYPE void*

void mux_thread_stop()
{
  sem_wait( &mux_w_sem );
  mux_data.id = THREAD_STOP_SERVICE_ID;
  sem_post( &mux_r_sem );
}
    
#else
#error "Operating mode (standalone or thread) not defined"
#endif

RETVAL_TYPE mux_thread( void *pdata ) 
{
  int c, prev_sent = -1;
  int temp, sdata;
  int got_esc = 0;
  DATA data;
                                                                                     
  log_msg( "Starting service multiplexer on %u port(s) and RFS server\n", vport_num );
  // Main service thread
  while( 1 )
  {
    sem_wait( &mux_r_sem );
    data = mux_data;
    sem_post( &mux_w_sem );
    if( data.id == THREAD_STOP_SERVICE_ID )
      break;        
    if( data.id == TRANSPORT_SERVICE_ID )
    {
      // Read one byte, interpret it
      c = data.data;
      if( c != ESCAPE_CHAR )
      {
        if( c >= SERVICE_ID_FIRST && c <= SERVICE_ID_LAST )
          service_id_in = c;
        else if( c == FORCE_SID_CHAR && prev_sent != -1 )
        {
          // Re-transmit the last data AND the service ID
          transport_send_byte( service_id_out );
          if( prev_sent & ESC_MASK )
            transport_send_byte( ESCAPE_CHAR );
          transport_send_byte( prev_sent & 0xFF );
        }
        else
        {
          if( got_esc )
          {
            // Got an escape last time, check the char now (with the 5th bit flipped)
            c ^= ESCAPE_XOR_MASK;
            if( c != ESCAPE_CHAR && c != FORCE_SID_CHAR && ( c < SERVICE_ID_FIRST || c > SERVICE_ID_LAST ) )
            {
               log_err( "Protocol error: invalid escape sequence\n" );
               break;
            }
            got_esc = 0;
          }  
          if( service_id_in == -1 )
          {
            transport_send_byte( FORCE_SID_CHAR );
            log_msg( "Requested resend of service ID\n" );
          }
          else
            byte_send( threads + service_id_in - SERVICE_ID_FIRST, c );
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
      prev_sent = sdata;
      // Then send the actual data byte, escaping it if needed
      if( sdata == ESCAPE_CHAR || sdata == FORCE_SID_CHAR || ( sdata >= SERVICE_ID_FIRST && sdata <= SERVICE_ID_LAST ) )
      {
        transport_send_byte( ESCAPE_CHAR );
        transport_send_byte( ( u8 )sdata ^ ESCAPE_XOR_MASK );
        prev_sent = ESC_MASK | ( ( u8 )sdata ^ ESCAPE_XOR_MASK );
      }
      else
        transport_send_byte( sdata );
      service_id_out = temp;
    }
  }
  mux_thread_running = 0;
  return RETVAL;
}

#ifdef MUX_STANDALONE_MODE
int main( int argc, char **argv )
{
  if( mux_init( argc, argv ) )
    return RETVAL;
  return mux_thread( NULL );
}
#endif
