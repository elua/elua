#include "build.h"
#ifdef BUILD_UIP

// UIP "helper" for eLua
// Implements the eLua specific UIP application

#include "elua_uip.h"
#include "elua_net.h"
#include "type.h"
#include "uip.h"
#include "uip_arp.h"
#include "platform.h"
#include "utils.h"
#include "uip-split.h"
#include <string.h>

// [REMOVE]
// Also remove any printf()s here and MYLOGs
#include <stdio.h>
#include "hw_types.h"
#include "hw_uart.h"
#include "hw_memmap.h"
#include "usart.h"
#define MYLOG( x ) UARTCharPut( UART0_BASE, x )

// UIP send buffer
extern void* uip_sappdata;



// *****************************************************************************
// Platform independenet eLua UIP "main loop" implementation

// Timers
static u32 periodic_timer, arp_timer;

// Macro for accessing the Ethernet header information in the buffer.
#define BUF                     ((struct uip_eth_hdr *)&uip_buf[0])

// UIP Timers (in ms)
#define UIP_PERIODIC_TIMER_MS   500
#define UIP_ARP_TIMER_MS        10000

// This is called from uip_split_output
void tcpip_output()
{
  platform_eth_send_packet( uip_buf, uip_len );
}

static void device_driver_send()
{
  uip_split_output();  
}

// This gets called on both Ethernet RX interrupts and timer requests,
// but it's called only from the Ethernet interrupt handler
void elua_uip_mainloop()
{
  u32 temp, packet_len;

  // Increment uIP timers
  temp = platform_eth_get_elapsed_time();
  periodic_timer += temp;
  arp_timer += temp;  

  // Check for an RX packet and read it
  if( ( packet_len = platform_eth_get_packet_nb( uip_buf, sizeof( uip_buf ) ) ) > 0 )
  {
    // Set uip_len for uIP stack usage.
    uip_len = ( unsigned short )packet_len;

    // Process incoming IP packets here.
    if( BUF->type == htons( UIP_ETHTYPE_IP ) )
    {
      uip_arp_ipin();
      uip_input();

      // If the above function invocation resulted in data that
      // should be sent out on the network, the global variable
      // uip_len is set to a value > 0.
      if( uip_len > 0 )
      {
        uip_arp_out();
        device_driver_send();
      }
    }

    // Process incoming ARP packets here.
    else if( BUF->type == htons( UIP_ETHTYPE_ARP ) )
    {
      uip_arp_arpin();

      // If the above function invocation resulted in data that
      // should be sent out on the network, the global variable
      // uip_len is set to a value > 0.
      if( uip_len > 0 )
        device_driver_send();
    }
  }
  
  for( temp = 0; temp < UIP_UDP_CONNS; temp ++ )
  {
    // Poll connection
    uip_poll_conn_num( temp );
    if( uip_len > 0 )
    {
      uip_arp_out();
      device_driver_send();
    }
  }
  
  // Process TCP/IP Periodic Timer here.
  if( periodic_timer >= UIP_PERIODIC_TIMER_MS )
  {
    periodic_timer = 0;
    for( temp = 0; temp < UIP_CONNS; temp ++ )
    {
      uip_periodic( temp );

      // If the above function invocation resulted in data that
      // should be sent out on the network, the global variable
      // uip_len is set to a value > 0.
      if( uip_len > 0 )
      {
        uip_arp_out();
        device_driver_send();
      }
    }

#if UIP_UDP
    for( temp = 0; temp < UIP_UDP_CONNS; temp ++ )
    {
      uip_udp_periodic( temp );

      // If the above function invocation resulted in data that
      // should be sent out on the network, the global variable
      // uip_len is set to a value > 0.
      if( uip_len > 0 )
      {
        uip_arp_out();
        device_driver_send();
      }
    }
#endif // UIP_UDP
  }  
  
  // Process ARP Timer here.
  if( arp_timer >= UIP_ARP_TIMER_MS )
  {
    arp_timer = 0;
    uip_arp_timer();
  }  
}

// *****************************************************************************
// Console over Ethernet support

#ifdef BUILD_CON_TCP

// TELNET specific data
#define TELNET_IAC_CHAR        255
#define TELNET_IAC_3B_FIRST    251
#define TELNET_IAC_3B_LAST     254
#define TELNET_SB_CHAR         250
#define TELNET_SE_CHAR         240
#define TELNET_EOF             236

// The telnet socket number
static int elua_uip_telnet_socket = -1;

// Utility function for TELNET: parse input buffer, skipping over
// TELNET specific sequences
// Returns the length of the buffer after processing
static elua_net_size elua_uip_telnet_parse_input( struct elua_uip_state* s, elua_net_size size )
{
  u8* dptr = ( u8* )uip_appdata;
  int skip;
  
  // Traverse the input buffer, skipping over TELNET sequences
  while( ( size > 0 ) && ( dptr < ( u8* )uip_appdata + uip_datalen() ) )
  {
    if( *dptr != TELNET_IAC_CHAR ) // regular char, copy it to buffer
      *s->ptr ++ = *dptr ++;
    else
    {
      // Control sequence: 2 or 3 bytes?
      if( ( dptr[ 1 ] >= TELNET_IAC_3B_FIRST ) && ( dptr[ 1 ] <= TELNET_IAC_3B_LAST ) )
        skip = 3;
      else
      {
        // Check EOF indication
        if( dptr[ 1 ] == TELNET_EOF )
        {
          *s->ptr ++ = STD_CTRLZ_CODE;
          size ++;
        }
        skip = 2;
      }
      dptr += skip;
      size = UMAX( 0, size - skip );
    }
  } 
  return size;
}

// Utility function for TELNET: prepend all '\n' with '\r' in buffer
// Returns actual len
// It is assumed that the buffer is "sufficiently smaller" than the UIP
// buffer (which is true for the default configuration: 128 bytes buffer
// in Newlib for stdin/stdout, more than 1024 bytes UIP buffer)
static elua_net_size elua_uip_telnet_prep_send( const char* src, elua_net_size size )
{
  elua_net_size actsize = size, i;
  char* dest = ( char* )uip_sappdata;
    
  for( i = 0; i < size; i ++ )
  {
    if( *src == '\n' )
    {
      *dest ++ = '\r';
      actsize ++;
    } 
    *dest ++ = *src ++;
  }
  return actsize;
}

#endif // #ifdef BUILD_CON_TCP

// *****************************************************************************
// eLua UIP application (used to implement the eLua TCP/IP services)

// Special handling for "accept"
volatile static u8 elua_uip_accept_request;
volatile static int elua_uip_accept_sock;

void elua_uip_appcall()
{
  struct elua_uip_state *s;
  elua_net_size temp;
  int sockno;
    
  s = ( struct elua_uip_state* )&( uip_conn->appstate );
  // Need to find the actual socket location, since UIP doesn't provide this ...
  for( temp = 0; temp < UIP_CONNS; temp ++ )
    if( uip_conns + temp == uip_conn )
      break;
  sockno = ( int )temp;

  if( uip_connected() )
  {
    // [TODO] handle locally initiated connections too (via uip_connect)
#ifdef BUILD_CON_TCP    
    if( uip_conn->lport == HTONS( ELUA_NET_TELNET_PORT ) ) // special case: telnet server
      elua_uip_telnet_socket = sockno;
    else
#endif
    if( elua_uip_accept_request )
    {
      elua_uip_accept_sock = sockno;
      elua_uip_accept_request = 0;
    }
    return;
  }

  if( s->state == ELUA_UIP_STATE_IDLE )
    return;
    
  // Do we need to read?
  if( s->state == ELUA_UIP_STATE_RECV )
  {
    // Re-enable data transfer on the socket and change state
    s->state = ELUA_UIP_STATE_RECV_2;
    uip_restart();
    return;
  }
  
  if( uip_aborted() || uip_timedout() || uip_closed() )
  {
    // Signal this error
    s->res = uip_aborted() ? ELUA_UIP_ERR_ABORTED : ( uip_timedout() ? ELUA_UIP_ERR_TIMEDOUT : ELUA_UIP_ERR_CLOSED );
#ifdef BUILD_CON_TCP    
    if( sockno == elua_uip_telnet_socket )
      elua_uip_telnet_socket = -1;      
#endif    
    s->state = ELUA_UIP_STATE_IDLE;
    return;
  }
       
  // Handle data send  
  if( ( uip_acked() || uip_rexmit() || uip_poll() ) && ( s->state == ELUA_UIP_STATE_SEND ) )
  {
    // Special translation for TELNET: prepend all '\n' with '\r'
    // We write directly in UIP's buffer 
    if( uip_acked() )
    {
      // Send next part of the buffer (if needed)
      if( s->len < uip_mss() ) // end of transmission
      { 
        s->len = 0;
        s->state = ELUA_UIP_STATE_IDLE;
      }
      else
      {
        s->len -= uip_conn->len;
        s->ptr += uip_conn->len;
      }
    }
    if( s->len > 0 ) // need to (re)transmit?
    {
#ifdef BUILD_CON_TCP    
      if( sockno == elua_uip_telnet_socket )
      {
        temp = elua_uip_telnet_prep_send( s->ptr, s->len );
        uip_send( uip_sappdata, temp );
      }
      else
#endif      
        uip_send( s->ptr, s->len );
    }
    return;
  }
  
  // Handle close
  if( s->state == ELUA_UIP_STATE_CLOSE )
  {
    uip_close();
    s->state = ELUA_UIP_STATE_IDLE;
  }
          
  // Handle data receive  
  if( uip_newdata() )
  {
    if( s->state == ELUA_UIP_STATE_RECV_2 )
    {
      temp = uip_datalen();
#ifdef BUILD_CON_TCP      
      if( sockno == elua_uip_telnet_socket )
      {
        if( ( temp = elua_uip_telnet_parse_input( s, temp ) ) == 0 )
          return;
      }
      else
#endif      
        memcpy( s->ptr, uip_appdata, UMIN( s->len, temp ) );
      s->len = UMIN( s->len, temp );  
      if( s->len < temp )
        s->res = ELUA_UIP_ERR_OVERFLOW;
      uip_stop();
      s->state = ELUA_UIP_STATE_IDLE;
    }
    else
      uip_stop();
    return;
  }
}

// Init application
void elua_uip_init()
{
#ifdef BUILD_CON_TCP
  uip_listen( HTONS( ELUA_NET_TELNET_PORT ) );
#endif  
}

// *****************************************************************************
// eLua TCP/IP services (from elua_net.h)

#define ELUA_UIP_IS_SOCK_OK( sock ) ( sock >= 0 && sock < UIP_CONNS )

int elua_net_socket( int type )
{
  int i;
  struct uip_conn* pconn;
  
  // [TODO] add UDP support at some point.
  if( type == ELUA_NET_SOCK_DGRAM )
    return -1;
  
  platform_cpu_disable_interrupts();
  // Iterate through the list of connections, looking for a free one
  for( i = 0; i < UIP_CONNS; i ++ )
  {
    pconn = uip_conns + i;
    if( pconn->tcpstateflags == UIP_CLOSED )
    { 
      // Found a free connection, reserve it for later use
      uip_conn_reserve( i );
      break;
    }
  }
  platform_cpu_enable_interrupts();
  return i == UIP_CONNS ? -1 : i;
}

// Send data
elua_net_size elua_net_send( int s, const void* buf, elua_net_size len )
{
  volatile struct elua_uip_state *pstate = ( volatile struct elua_uip_state* )&( uip_conns[ s ].appstate );
  
  if( !ELUA_UIP_IS_SOCK_OK( s ) || !uip_conn_active( s ) )
    return -1;
  if( len == 0 )
    return 0;
  pstate->ptr = ( char* )buf;
  pstate->len = len;
  pstate->res = ELUA_UIP_OK;
  pstate->state = ELUA_UIP_STATE_SEND;  
  platform_eth_force_interrupt();
  while( pstate->state != ELUA_UIP_STATE_IDLE );
  return len - pstate->len;
}

// Receive data
elua_net_size elua_net_recv( int s, void* buf, elua_net_size maxsize )
{
  volatile struct elua_uip_state *pstate = ( volatile struct elua_uip_state* )&( uip_conns[ s ].appstate );
  
  if( !ELUA_UIP_IS_SOCK_OK( s ) || !uip_conn_active( s ) )
    return -1;
  if( maxsize == 0 )
    return 0;
  pstate->ptr = ( char* )buf;
  pstate->len = maxsize;
  pstate->res = ELUA_UIP_OK;  
  pstate->state = ELUA_UIP_STATE_RECV;
  while( pstate->state != ELUA_UIP_STATE_IDLE );
  return pstate->len;
}

// Return the socket associated with the "telnet" application (or -1 if it does
// not exist). The socket only exists if a client connected to the board.
int elua_net_get_telnet_socket()
{
  int res = -1;
  
#ifdef BUILD_CON_TCP  
  if( elua_uip_telnet_socket != -1 )
    if( uip_conn_active( elua_uip_telnet_socket ) )
      res = elua_uip_telnet_socket;
#endif      
  return res;
}

// Close socket
int elua_net_close( int s )
{
  volatile struct elua_uip_state *pstate = ( volatile struct elua_uip_state* )&( uip_conns[ s ].appstate );  
  
  if( !ELUA_UIP_IS_SOCK_OK( s ) || !uip_conn_active( s ) )
    return -1;
  pstate->res = ELUA_UIP_OK;    
  pstate->state = ELUA_UIP_STATE_CLOSE;
  platform_eth_force_interrupt();
  while( pstate->state != ELUA_UIP_STATE_IDLE );
  return 0;
}

// Get last error on specific socket
int elua_net_get_last_err( int s )
{
  volatile struct elua_uip_state *pstate = ( volatile struct elua_uip_state* )&( uip_conns[ s ].appstate );  
  
  if( !ELUA_UIP_IS_SOCK_OK( s ) )
    return -1;
  return pstate->res;
}

// Accept a connection on the given port, return its socket id
int elua_accept( u16 port )
{
#ifdef BUILD_CON_TCP
  if( port == ELUA_NET_TELNET_PORT )
    return -1;
#endif  
  platform_cpu_disable_interrupts();
  uip_unlisten( htons( port ) );
  uip_listen( htons( port ) );
  platform_cpu_enable_interrupts();
  elua_uip_accept_sock = -1;
  elua_uip_accept_request = 1;
  while( elua_uip_accept_request );
  return elua_uip_accept_sock;
}

#endif // #ifdef BUILD_UIP
