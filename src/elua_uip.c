#include "platform_conf.h"
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
#include "dhcpc.h"
#include "resolv.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

// UIP send buffer
extern void* uip_sappdata;

// Global "configured" flag
static volatile u8 elua_uip_configured;

// "Link changed" call back
static p_elua_net_state_cb elua_uip_state_cb;

#undef BUILD_CON_TCP
#define BUILD_CON_TCP

// ****************************************************************************
// Logging

#ifdef TCPIP_LOGS
static void elua_uip_log( const char *fmt, ... )
{
#ifndef BUILD_CON_TCP
  va_list ap;

  va_start( ap, fmt );
  printf( "[elua_uip] " );
  vprintf( fmt, ap );
  va_end( ap );
#endif // #ifndef BUILD_CON_TCP
}

static const char* elua_uip_iptostr( const u16 *pip )
{
  elua_net_ip res = { 0 };
  static char strip[ 16 ];

  uip_ipaddr_copy( ( u16* )&res, pip );
  sprintf( strip, "%d.%d.%d.%d", ( int )res.ipbytes[ 0 ], ( int )res.ipbytes[ 1 ],
    ( int )res.ipbytes[ 2 ], ( int )res.ipbytes[ 3 ] );
  return strip;
}
#else // #ifdef TCPIP_LOGS
#define elua_uip_log( fmt, ... )
#endif // #ifdef TCPIP_LOGS

// *****************************************************************************
// Platform independenet eLua UIP "main loop" implementation

// Timers
static u32 periodic_timer, arp_timer;

// Macro for accessing the Ethernet header information in the buffer.
#define BUF                     ((struct uip_eth_hdr *)&uip_buf[0])
#define UDPBUF                  ((struct uip_udpip_hdr *)&uip_buf[UIP_LLH_LEN])

// UIP Timers (in ms)
#define UIP_PERIODIC_TIMER_MS   500
#define UIP_ARP_TIMER_MS        10000

static void device_driver_send()
{
  platform_eth_send_packet( uip_buf, uip_len );
}

// This gets called on both Ethernet RX interrupts and timer requests,
// but it's called only from the Ethernet interrupt handler
void elua_uip_mainloop()
{
  u32 temp, packet_len;

  if( platform_eth_get_link_status() == PLATFORM_ETH_LINK_DOWN )
    return;

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
  
  // Process TCP/IP Periodic Timer here.
  // Also process the "force interrupt" events (platform_eth_force_interrupt)
  if( periodic_timer >= UIP_PERIODIC_TIMER_MS )
  {
    periodic_timer = 0;
    uip_set_forced_poll( 0 );
  }
  else
    uip_set_forced_poll( 1 );
  for( temp = 0; temp < UIP_CONNS; temp ++ )
  {
    uip_periodic( temp );
    // If the above function invocation resulted in data that
    // should be sent out on the network, the global variable
    // uip_len is set to a value > 0.
    if( uip_len > 0 )
    {
      if( uip_arp_out() ) // packet was replaced with ARP, need to resend
        uip_conns[ temp ].appstate.state = ELUA_UIP_STATE_RETRY;
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
      if( uip_arp_out() ) // packet was replaced with ARP, need to resend
        uip_udp_conns[ temp ].appstate.state = ELUA_UIP_STATE_RETRY;
      device_driver_send();
    }
  }
#endif // UIP_UDP
  
  // Process ARP Timer here.
  if( arp_timer >= UIP_ARP_TIMER_MS )
  {
    arp_timer = 0;
    uip_arp_timer();
  }  
}

// *****************************************************************************
// DHCP callback

#ifdef BUILD_DHCPC
static void elua_uip_conf_static();

void dhcpc_configured(const struct dhcpc_state *s)
{
  if( s->ipaddr[ 0 ] != 0 )
  {
    elua_uip_log( "Got DHCP IP: %s\n", elua_uip_iptostr( s->ipaddr ) );
    elua_uip_log( "    Netmask: %s\n", elua_uip_iptostr( s->netmask ) );
    elua_uip_log( "        DNS: %s\n", elua_uip_iptostr( s->dnsaddr ) );
    elua_uip_log( "         GW: %s\n", elua_uip_iptostr( s->default_router ) );
    uip_sethostaddr( s->ipaddr );
    uip_setnetmask( s->netmask ); 
    uip_setdraddr( s->default_router );     
    resolv_conf( ( u16_t* )s->dnsaddr );
    elua_uip_configured = 1;
    if( elua_uip_state_cb )
    {
      elua_uip_log( "Invoking sate changed callback" );
      elua_uip_state_cb( ELUA_NET_STATE_UP );
    }
  }
  else
    elua_uip_conf_static();
}
#endif

// *****************************************************************************
// DNS callback

#ifdef BUILD_DNS
volatile static int elua_resolv_req_done;
static elua_net_ip elua_resolv_ip;

void resolv_found( char *name, u16_t *ipaddr )
{
  if( !ipaddr )
    elua_resolv_ip.ipaddr = 0;
  else
    uip_ipaddr_copy( ( u16* )&elua_resolv_ip, ipaddr );
  elua_resolv_req_done = 1;
}
#endif

// *****************************************************************************
// Console over Ethernet support

#ifdef BUILD_CON_TCP

// TELNET specific data
#define TELNET_IAC_CHAR         255
#define TELNET_IAC_3B_FIRST     251
#define TELNET_IAC_3B_LAST      254
#define TELNET_SB_CHAR          250
#define TELNET_SE_CHAR          240
#define TELNET_EOF              236

#define ELUA_UIP_TELNET_SOCKET  0

// Utility function for TELNET: parse input buffer, skipping over
// TELNET specific sequences
// Returns the length of the buffer after processing
static void elua_uip_telnet_handle_input( volatile struct elua_uip_state* s )
{
  u8 *dptr = ( u8* )uip_appdata;
  char *orig = ( char* )s->ptr;
  int skip;
  elua_net_size maxsize = s->len;
  
  // Traverse the input buffer, skipping over TELNET sequences
  while( ( dptr < ( u8* )uip_appdata + uip_datalen() ) && ( s->ptr - orig < s->len ) )
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
          *s->ptr ++ = STD_CTRLZ_CODE;
        skip = 2;
      }
      dptr += skip;
    }
  } 
  if( s->ptr > orig )
  {
    s->res = ELUA_NET_ERR_OK;
    s->len = maxsize - ( s->ptr - orig );
    s->state = ELUA_UIP_STATE_IDLE;
  }
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
#else // #ifdef BUILD_CON_TCP
#define ELUA_UIP_TELNET_SOCKET  ( -1 )
#endif // #ifdef BUILD_CON_TCP

// *****************************************************************************
// eLua UIP application (used to implement the eLua TCP/IP services)

// Special handling for "accept"
volatile static u8 elua_uip_accept_request;
volatile static elua_net_ip elua_uip_accept_remote;

// Global helper for read: transfer data either directly in memory or in Lua buffer
static void eluah_uip_generic_read( void *dest, const void* src, unsigned len, int with_buffer )
{
  if( with_buffer )
    luaL_addlstring( ( luaL_Buffer* )dest, src, len );
  else
    memcpy( dest, src, len );
}

// Read data helper
static void eluah_uip_read_data( volatile struct elua_uip_state *s, elua_net_size temp )
{
  eluah_uip_generic_read( s->ptr, uip_appdata, temp, s->res );
  s->len -= temp;
}

// Read to buffer helper
static void eluah_uip_read_to_buffer( volatile struct elua_uip_state *s )
{
  elua_net_size available = s->buf_total - s->buf_crt;
  int total = uip_datalen(), temp;

  if( total > available )
  {
    total = available;
    s->res = ELUA_NET_ERR_OVERFLOW;
  }
  if( available == 0 || total == 0 )
    return;
  // First transfer: widx -> end of buffer
  temp = UMIN( total, s->buf_total - s->buf_widx );
  memcpy( s->buf + s->buf_widx, ( const char* )uip_appdata, temp );
  s->buf_widx += temp;
  if( s->buf_widx == s->buf_total )
  {
    s->buf_widx = 0;
    if( temp < total ) // second transfer: from the start of buffer
    {
      memcpy( s->buf, ( const char* )uip_appdata + temp, total - temp );
      s->buf_widx += total - temp;
    }
  }
  s->buf_crt += total;
}

void elua_uip_appcall()
{
  volatile struct elua_uip_state *s;
  elua_net_size temp;
  int sockno;
  
  // If uIP is not yet configured (DHCP response not received), do nothing
  if( !elua_uip_configured )
    return;
  s = ( struct elua_uip_state* )&( uip_conn->appstate );
  sockno = uip_conn - uip_conns;

  if( uip_connected() )
  {
    platform_s_uart_send( CON_UART_ID, '%' );
    if( sockno != ELUA_UIP_TELNET_SOCKET )
    {
      if( elua_uip_accept_request )
      {
        uip_ipaddr_copy( ( u16* )&elua_uip_accept_remote.ipaddr, uip_conn->ripaddr );
        elua_uip_accept_request = 0;
      }
      else if( s->state == ELUA_UIP_STATE_CONNECT )
        s->state = ELUA_UIP_STATE_IDLE;
    }
    else
      platform_s_uart_send( CON_UART_ID, 'x' );
  }

//  if( s->state == ELUA_UIP_STATE_IDLE )
  //  return;
 
  if( uip_aborted() || uip_timedout() || uip_closed() )
  {
#ifdef BUILD_CON_TCP
    if( sockno == ELUA_UIP_TELNET_SOCKET ) // reinitialize telnet socket
    {
      platform_s_uart_send( CON_UART_ID, '^' );
      uip_conn_mark_accept( ELUA_UIP_TELNET_SOCKET );
      uip_unlisten( htons( ELUA_NET_TELNET_PORT ) );
      uip_listen( htons( ELUA_NET_TELNET_PORT ) ); 
    }
    else
#endif
    if( !uip_closed() || s->state != ELUA_UIP_STATE_CLOSE_ACK ) // differentiate between error and close request
      s->res = uip_aborted() ? ELUA_NET_ERR_ABORTED : ( uip_timedout() ? ELUA_NET_ERR_TIMEDOUT : ELUA_NET_ERR_CLOSED );
    s->state = ELUA_UIP_STATE_IDLE;
    //return;
  }

  // Handle data receive  
  if( uip_newdata() && uip_datalen() > 0 )
  {
    elua_uip_log( "%d bytes recvd on %d\n", uip_datalen(), sockno );
    if( s->socket_cb )
    {
      elua_net_ip ip = { 0 };
      s->socket_cb( sockno, uip_appdata, uip_datalen(), ip, 0 );
    }
    else if( s->buf )
      eluah_uip_read_to_buffer( s );
    else if( s->state == ELUA_UIP_STATE_RECV )
    {
#ifdef BUILD_CON_TCP      
      if( sockno == ELUA_UIP_TELNET_SOCKET )
      {
        platform_s_uart_send( CON_UART_ID, '@' );
        elua_uip_telnet_handle_input( s );
        // [TODO] TCP: is the below 'return' correct?
        return;
      }
#endif   
      sockno = ELUA_NET_ERR_OK;
      // Check overflow
      if( s->len < uip_datalen() )
      {
        sockno = ELUA_NET_ERR_OVERFLOW;   
        temp = s->len;
      }
      else
        temp = uip_datalen();

      eluah_uip_read_data( s, temp );
      
      s->res = sockno;
      s->state = ELUA_UIP_STATE_IDLE;
    }
  }
      
  // Handle data send  
  if( ( uip_acked() || uip_rexmit() || uip_poll() ) && ( s->state == ELUA_UIP_STATE_SEND ) )
  {
    // Special translation for TELNET: prepend all '\n' with '\r'
    // We write directly in UIP's buffer 
    if( uip_acked() )
    {
      temp = UMIN( s->len, uip_mss() );
      s->len -= temp;
      s->ptr += temp;
    }
    if( s->len > 0 ) // need to (re)transmit?
    {
#ifdef BUILD_CON_TCP
      if( sockno == ELUA_UIP_TELNET_SOCKET )
      {
        temp = elua_uip_telnet_prep_send( s->ptr, s->len );
        uip_send( uip_sappdata, temp );
      }
      else
#endif      
        uip_send( s->ptr, UMIN( s->len, uip_mss() ) );
    }
    else
      s->state = ELUA_UIP_STATE_IDLE;
    //return;
  }
  
  // Handle close
  if( s->state == ELUA_UIP_STATE_CLOSE )
  {
    uip_close();
    s->state = ELUA_UIP_STATE_CLOSE_ACK;
  }       
}

static void elua_uip_conf_static()
{
  uip_ipaddr_t ipaddr;

  if( platform_eth_get_link_status() == PLATFORM_ETH_LINK_DOWN )
  {
    elua_uip_log( "Unable to configure static IP, link is down\n" );
    return;
  }
  elua_uip_log( "Static IP configuration: %d.%d.%d.%d\n", ELUA_CONF_IPADDR0, ELUA_CONF_IPADDR1, ELUA_CONF_IPADDR2, ELUA_CONF_IPADDR3 );
  elua_uip_log( "                Netmask: %d.%d.%d.%d\n", ELUA_CONF_NETMASK0, ELUA_CONF_NETMASK1, ELUA_CONF_NETMASK2, ELUA_CONF_NETMASK3 );
  elua_uip_log( "                    DNS: %d.%d.%d.%d\n", ELUA_CONF_DNS0, ELUA_CONF_DNS1, ELUA_CONF_DNS2, ELUA_CONF_DNS3 );
  elua_uip_log( "                     GW: %d.%d.%d.%d\n", ELUA_CONF_DEFGW0, ELUA_CONF_DEFGW1, ELUA_CONF_DEFGW2, ELUA_CONF_DEFGW3 );
  uip_ipaddr( ipaddr, ELUA_CONF_IPADDR0, ELUA_CONF_IPADDR1, ELUA_CONF_IPADDR2, ELUA_CONF_IPADDR3 );
  uip_sethostaddr( ipaddr );
  uip_ipaddr( ipaddr, ELUA_CONF_NETMASK0, ELUA_CONF_NETMASK1, ELUA_CONF_NETMASK2, ELUA_CONF_NETMASK3 );
  uip_setnetmask( ipaddr ); 
  uip_ipaddr( ipaddr, ELUA_CONF_DEFGW0, ELUA_CONF_DEFGW1, ELUA_CONF_DEFGW2, ELUA_CONF_DEFGW3 );
  uip_setdraddr( ipaddr );    
  uip_ipaddr( ipaddr, ELUA_CONF_DNS0, ELUA_CONF_DNS1, ELUA_CONF_DNS2, ELUA_CONF_DNS3 );
  resolv_conf( ipaddr );  
  elua_uip_configured = 1;
  if( elua_uip_state_cb )
  {
    elua_uip_log( "Invoking state changed callback\n" );
    elua_uip_state_cb( ELUA_NET_STATE_UP );
  }
}

// This part of initializtion always takes place when a link is detected
static void elua_uip_linkinit()
{
  if( platform_eth_get_link_status() == PLATFORM_ETH_LINK_DOWN )
    return;
  
#ifdef BUILD_DHCPC
  dhcpc_init( uip_ethaddr.addr, sizeof( uip_ethaddr ) );
  dhcpc_request();
#else
  elua_uip_conf_static();
#endif
  
  resolv_init();
}

// Init application
void elua_net_init( void *pdata )
{
  const struct uip_eth_addr *paddr = ( const struct uip_eth_addr* )pdata;

  // Set hardware address
  uip_setethaddr( (*paddr) );

  uip_init();
  uip_arp_init();
  elua_uip_configured = 0;

#ifdef BUILD_CON_TCP
  // Reserve the first socket for TELNET now and keep it like that
  uip_conn_mark_accept( ELUA_UIP_TELNET_SOCKET );
  uip_unlisten( htons( ELUA_NET_TELNET_PORT ) );
  uip_listen( htons( ELUA_NET_TELNET_PORT ) );
#endif
  
  // Low level initialization
  elua_uip_linkinit();
}

// Helper for forced closing sockets
static void eluah_net_forced_close( struct elua_uip_state *pstate )
{
  if( pstate->buf )
    free( pstate->buf );
  memset( pstate, 0, sizeof( *pstate ) );
}

void elua_net_link_changed()
{
  int state = platform_eth_get_link_status();
  unsigned i;

  elua_uip_log( "elua_net_link_changed called, link is %s.\n", state == PLATFORM_ETH_LINK_UP ? "UP" : "DOWN" );
  if( state == PLATFORM_ETH_LINK_DOWN )
  {
    uip_init();
    uip_arp_init();
    elua_uip_configured = 0;
    // Clean all TCP sockets
    for( i = 0; i < UIP_CONNS; i ++ )
      eluah_net_forced_close( ( struct elua_uip_state* )&( uip_conns[ i ].appstate ) );
    // And all UDP sockets
    for( i = 0; i < UIP_UDP_CONNS; i ++ )
      eluah_net_forced_close( ( struct elua_uip_state* )&( uip_udp_conns[ i ].appstate ) );
    // Call state changed callback if available
    if( elua_uip_state_cb )
      elua_uip_state_cb( ELUA_NET_STATE_DOWN );
  }
  else
    elua_uip_linkinit();
}

// *****************************************************************************
// eLua UIP UDP application (used for the DHCP client and the DNS resolver)

extern int dhcpc_socket;
extern int resolv_socket;

void elua_uip_udp_appcall()
{
  volatile struct elua_uip_state *s;
  elua_net_size temp;
  int sockno;

  if( platform_eth_get_link_status() == PLATFORM_ETH_LINK_DOWN )
    return;
  
  s = ( struct elua_uip_state* )&( uip_udp_conn->appstate );
  sockno = uip_udp_conn - uip_udp_conns;

  // Is this the DHCP socket?
  if( sockno == dhcpc_socket )
  {
    dhcpc_appcall();
    return;
  }

   // If uIP is not yet configured (DHCP response not received), do nothing
  if( !elua_uip_configured )
    return;

  // Is this the resolver (DNS client) socket?
  if( sockno == resolv_socket )
  {
    resolv_appcall();
    return;
  }

  // Must be an application socket, so check its state
  if( uip_newdata() && uip_datalen() > 0 ) // handle data receive
  {
    if( s->socket_cb )
    {
      elua_net_ip ip;
      uip_ipaddr_copy( ( u16* )&ip, UDPBUF->srcipaddr );
      s->socket_cb( sockno, uip_appdata, uip_datalen(), ip, HTONS( uip_udp_conn->rport ) );
    }
    else if( s->buf )
      eluah_uip_read_to_buffer( s );
    else if( s->state == ELUA_UIP_STATE_RECV )
    {
      sockno = ELUA_NET_ERR_OK;
      // Check overflow
      if( s->len < uip_datalen() )
      {
        sockno = ELUA_NET_ERR_OVERFLOW;   
        temp = s->len;
      }
      else
        temp = uip_datalen();

      eluah_uip_read_data( s, temp );
      
      uip_ipaddr_copy( uip_udp_conn->ripaddr, UDPBUF->srcipaddr );
      s->res = sockno;
      s->state = ELUA_UIP_STATE_IDLE;
    }
  }
 
  // Anything to send ?
  if( uip_poll() && s->state == ELUA_UIP_STATE_SEND )
  {
    uip_send( s->ptr, s->len );
    s->len = 0;
    s->state = ELUA_UIP_STATE_IDLE;
  }

  if( s->state == ELUA_UIP_STATE_CLOSE ) // handle close (trivial)
  {
    uip_udp_conn->lport = 0;
    s->state = ELUA_UIP_STATE_IDLE;
  }
}

// *****************************************************************************
// eLua TCP/IP services (from elua_net.h)

#define ELUA_UIP_IS_SOCK_OK( sock ) ( elua_uip_configured && !ELUA_UIP_IS_UDP( sock ) && sock >= 0 && sock < UIP_CONNS )
#define ELUA_UIP_IS_UDP_SOCK_OK( sock ) ( elua_uip_configured && ELUA_UIP_IS_UDP( sock ) && ELUA_UIP_FROM_UDP( sock ) >= 0 && ELUA_UIP_FROM_UDP( sock ) < UIP_UDP_CONNS )

static void elua_prep_socket_state( volatile struct elua_uip_state *pstate, void* buf, elua_net_size len, u8 res, u8 state )
{  
  pstate->ptr = ( char* )buf;
  pstate->len = len;
  pstate->res = res;
  pstate->state = state;
}

p_elua_net_state_cb elua_net_set_state_cb( p_elua_net_state_cb plink )
{
  p_elua_net_state_cb prev = elua_uip_state_cb;

  elua_uip_state_cb = plink;
  return prev;
}

p_elua_net_state_cb elua_net_get_state_cb()
{
  return elua_uip_state_cb;
}

int elua_net_socket( int type )
{
  int i;
  struct uip_conn* pconn;
  struct uip_udp_conn* pudp;
  volatile struct elua_uip_state *pstate = NULL;

  if( !elua_uip_configured )
    return ELUA_NET_INVALID_SOCKET;
  platform_eth_set_interrupt( PLATFORM_ETH_INT_DISABLE );
  if( type == ELUA_NET_SOCK_DGRAM )
  {
    if( ( pudp = uip_udp_new( NULL, 0 ) ) == NULL )
      i = -1;
    else
    {
      pstate = ( volatile struct elua_uip_state* )&( pudp->appstate );
      i = ELUA_UIP_TO_UDP( pudp->connidx );
    }
  }
  else
  {
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
    if( i == UIP_CONNS )
      i = -1;
    else
      pstate = ( volatile struct elua_uip_state* )&( pconn->appstate );
  }
  platform_eth_set_interrupt( PLATFORM_ETH_INT_ENABLE );
  if( pstate )
  {
    memset( ( void* )pstate, 0, sizeof( *pstate ) );
    pstate->state = ELUA_UIP_STATE_IDLE;
    pstate->split = ELUA_NET_NO_SPLIT;
  }
  return i;
}

// Helper: get a socket and return a pointer to its state, also chage socket number if needed
// Returns socket type or -1 for error
static int eluah_get_socket_state( int *ps, volatile struct elua_uip_state **ppstate, int check_active )
{
  int res = ELUA_NET_SOCK_STREAM; 

  *ppstate = NULL;
  if( ELUA_UIP_IS_UDP( *ps ) ) // UDP socket
  {
    if( !ELUA_UIP_IS_UDP_SOCK_OK( *ps ) )
      return -1;
    *ps = ELUA_UIP_FROM_UDP( *ps );
    *ppstate = ( volatile struct elua_uip_state* )&( uip_udp_conns[ *ps ].appstate );
    res = ELUA_NET_SOCK_DGRAM;
  }
  else // TCP socket
  {
    if( !ELUA_UIP_IS_SOCK_OK( *ps ) )
      return -1;
    if( check_active && !uip_conn_active( *ps ) )
      return -1;
    *ppstate = ( volatile struct elua_uip_state* )&( uip_conns[ *ps ].appstate );
  }
  return res;
}

// Helper: check if the stack is still up, return 1 if it is,
// return 0 and set 'state' properly if it is not
static int eluah_check_stack_state( volatile struct elua_uip_state *pstate )
{
  if( elua_uip_configured )
    return 1;
  else
  {
    pstate->res = ELUA_NET_ERR_LINKDOWN;
    pstate->state = ELUA_UIP_STATE_IDLE;
    return 0;
  }
}

// Send data - internal helper (also works for 'sendto')
static elua_net_size elua_net_send_internal( int s, const void* buf, elua_net_size len, elua_net_ip remoteip, u16 remoteport, int is_sendto )
{
  volatile struct elua_uip_state *pstate;
  elua_net_size sentbytes = 0;
  int res;
 
  if( len == 0 )
    return 0;
  if( ( res = eluah_get_socket_state( &s, &pstate, 1 ) ) == -1 )
    return 0;
  // Check for valid operation (sendto on UDP, send on recv)
  if( ( ( res == ELUA_NET_SOCK_DGRAM ) && !is_sendto ) || ( ( res == ELUA_NET_SOCK_STREAM ) && is_sendto ) )
    return 0;
  if( res == ELUA_NET_SOCK_DGRAM )
  {
    uip_ipaddr_copy( uip_udp_conns[ s ].ripaddr, ( u16* )&remoteip );
    uip_udp_conns[ s ].rport = HTONS( remoteport ); 
  }
  // Send data in 'sendlimit' chunks
  while( 1 )
  {
    elua_prep_socket_state( pstate, ( void* )buf, len, ELUA_NET_ERR_OK, ELUA_UIP_STATE_SEND );
    platform_eth_force_interrupt();
    while( pstate->state != ELUA_UIP_STATE_IDLE && pstate->state != ELUA_UIP_STATE_RETRY && elua_uip_configured );
    if( !eluah_check_stack_state( pstate ) )
      break;
    if( pstate->state == ELUA_UIP_STATE_RETRY ) // resend the exact same packet again
      continue;
    sentbytes = len - pstate->len;
    break;
  }
  return sentbytes;
}

elua_net_size elua_net_send( int s, const void* buf, elua_net_size len )
{
  elua_net_ip dummy = { 0 };
  return elua_net_send_internal( s, buf, len, dummy, 0, 0 );
}

// Internal "read" function (also works for 'recvfrom' when 'p_remote_ip' and 'p_remote_port' are not NULL)
static elua_net_size elua_net_recv_internal( int s, void* buf, elua_net_size maxsize, unsigned timer_id, timer_data_type to_us, int with_buffer, elua_net_ip *p_remote_ip, u16 *p_remote_port, int is_recvfrom )
{
  volatile struct elua_uip_state *pstate;
  timer_data_type tmrstart = 0;
  elua_net_size readsize, readbytes, totread = 0;
  int res, socktype;
  u8 b;
 
  if( maxsize == 0 )
    return 0;
  if( ( socktype = eluah_get_socket_state( &s, &pstate, 1 ) ) == -1 )
    return 0;
  // Check for valid operation (recvfrom on UDP, recv on TCP)
  if( ( ( socktype == ELUA_NET_SOCK_DGRAM ) && !is_recvfrom ) || ( ( socktype == ELUA_NET_SOCK_STREAM ) && is_recvfrom ) )
    return 0;
  // Read data in packets of maximum 'readlimit' bytes
  if( to_us != PLATFORM_TIMER_INF_TIMEOUT )
    tmrstart = platform_timer_op( timer_id, PLATFORM_TIMER_OP_START, 0 );
  res = 0; // 'res' will keep the match status in split mode
  if( pstate->buf ) // this is a buffered read, look for data
  {
    while( 1 )
    {
      if( ( readbytes = pstate->buf_crt ) > 0 )
        break;    
      if( !eluah_check_stack_state( pstate ) )
        break;
      if( to_us == 0 || ( to_us != PLATFORM_TIMER_INF_TIMEOUT && platform_timer_get_diff_crt( timer_id, tmrstart ) >= to_us ) )
        break;
      if( socktype == ELUA_NET_SOCK_STREAM && !uip_conn_active( s ) )
        break;
    }
    if( readbytes > maxsize )
      readbytes = maxsize;
    if( readbytes > 0 )
    {
      // Got some data, copy it in our buffer
      // If the read operation uses split chars we read byte by byte until we find the split char or the end of buffer
      if( pstate->split != ELUA_NET_NO_SPLIT )
      {
        readsize = 0;
        while( readsize < readbytes && res == 0 )
        {
          b = pstate->buf[ pstate->buf_ridx ];
          if( with_buffer )
            luaL_addchar( ( luaL_Buffer* )buf, ( char )b );
          else              
            *( ( u8* )buf + readsize ) = b;
          if( ++ pstate->buf_ridx == pstate->buf_total )
            pstate->buf_ridx = 0;
          readsize ++;
          if( b == pstate->split )
            res = 1;
        }
      }
      else // split not needed, just copy the data in the output buffer
      {
        // First transfer: ridx -> end of buffer
        readsize = UMIN( readbytes, pstate->buf_total - pstate->buf_ridx );
        eluah_uip_generic_read( buf, pstate->buf + pstate->buf_ridx, readsize, with_buffer );
        pstate->buf_ridx += readsize;
        if( pstate->buf_ridx == pstate->buf_total )
        {
          pstate->buf_ridx = 0;
          if( readsize < readbytes ) // second transfer: from the start of buffer
          {
            eluah_uip_generic_read( with_buffer ? buf : ( u8* )buf + readsize, pstate->buf, readbytes - readsize, with_buffer );
            pstate->buf_ridx += readbytes - readsize;
          }
        }
        readsize = readbytes;
      }
    }
    else
      readsize = 0;
    if( elua_uip_configured )
    {
      platform_eth_set_interrupt( PLATFORM_ETH_INT_DISABLE );
      pstate->buf_crt -= readsize;
      pstate->res = readbytes > 0 ? ELUA_NET_ERR_OK : ELUA_NET_ERR_TIMEDOUT;
      platform_eth_set_interrupt( PLATFORM_ETH_INT_ENABLE );
    }
  }
  else // direct read
  {
    readsize = UMIN( is_recvfrom ? UIP_APPDATA_SIZE : uip_conns[ s ].mss, maxsize );
    elua_prep_socket_state( pstate, buf, readsize, with_buffer, ELUA_UIP_STATE_RECV );
    while( 1 )
    {
      if( pstate->state == ELUA_UIP_STATE_IDLE )
        break;
      if( !eluah_check_stack_state( pstate ) )
        break;
      if( to_us == 0 || ( to_us != PLATFORM_TIMER_INF_TIMEOUT && platform_timer_get_diff_crt( timer_id, tmrstart ) >= to_us ) )
      {
        platform_eth_set_interrupt( PLATFORM_ETH_INT_DISABLE );
        if( pstate->state != ELUA_UIP_STATE_IDLE )
        { 
          pstate->res = ELUA_NET_ERR_TIMEDOUT;
          pstate->state = ELUA_UIP_STATE_IDLE;
        }
        platform_eth_set_interrupt( PLATFORM_ETH_INT_ENABLE );
        break;
      }
      if( socktype == ELUA_NET_SOCK_STREAM && !uip_conn_active( s ) )
        break;
    }
    readbytes = readsize - pstate->len;
  }
  totread += readbytes;
  if( is_recvfrom )
  {
    *p_remote_port = HTONS( uip_udp_conns[ s ].rport );
    uip_ipaddr_copy( ( u16* )p_remote_ip, uip_udp_conns[ s ].ripaddr );
  }
  return totread;
}

// Receive data in buf, upto "maxsize" bytes, or upto the 'readto' character if it's not -1
elua_net_size elua_net_recv( int s, void* buf, elua_net_size maxsize, unsigned timer_id, timer_data_type to_us )
{
  return elua_net_recv_internal( s, buf, maxsize, timer_id, to_us, 0, NULL, NULL, 0 );
}

// Same thing, but with a Lua buffer as argument
elua_net_size elua_net_recvbuf( int s, luaL_Buffer* buf, elua_net_size maxsize, unsigned timer_id, timer_data_type to_us )
{
  return elua_net_recv_internal( s, buf, maxsize, timer_id, to_us, 1, NULL, NULL, 0 );
}

// Return the socket associated with the "telnet" application (or -1 if it does
// not exist). The socket only exists if a client connected to the board.
int elua_net_get_telnet_socket()
{
  int res = -1;
  
#ifdef BUILD_CON_TCP  
  if( elua_uip_configured && uip_conn_active( ELUA_UIP_TELNET_SOCKET ) )
    res = ELUA_UIP_TELNET_SOCKET;
#endif      
  return res;
}

// Close socket
int elua_net_close( int s )
{
  volatile struct elua_uip_state *pstate;
  int res;

  if( s == ELUA_UIP_TELNET_SOCKET )
    return 0;
  if( ( res = eluah_get_socket_state( &s, &pstate, 0 ) ) == -1 )
    return 0;   
  if( res == ELUA_NET_SOCK_STREAM && !uip_conn_active( s ) )
  {
    if( pstate->buf )
    {
      free( pstate->buf );
      pstate->buf = NULL;
      pstate->buf_total = pstate->buf_crt = 0;
    }
    return 1;
  }
  elua_prep_socket_state( pstate, NULL, 0, ELUA_NET_ERR_OK, ELUA_UIP_STATE_CLOSE );
  platform_eth_force_interrupt();
  while( pstate->state != ELUA_UIP_STATE_IDLE && elua_uip_configured );
  if( !eluah_check_stack_state( pstate ) )
    return 0;
  if( pstate->buf )
  {
    free( pstate->buf );
    pstate->buf = NULL;
    pstate->buf_total = pstate->buf_crt = 0;
  }
  pstate->split = ELUA_NET_NO_SPLIT;
  return pstate->res == ELUA_NET_ERR_OK ? 1 : 0;
}

// Get last error on specific socket
int elua_net_get_last_err( int s )
{
  volatile struct elua_uip_state *pstate;

  if( eluah_get_socket_state( &s, &pstate, 0 ) == -1 )
    return -1;
  return pstate->res;
}

// Sets the receive callback for the socket
void elua_net_set_socket_callback( int s, p_elua_net_socket_cb callback )
{
  volatile struct elua_uip_state *pstate;

  if( eluah_get_socket_state( &s, &pstate, 0 ) == -1 )
    return;
  pstate->socket_cb = callback;
}

// Set the "split char": recv/recvfrom will return if this char is
// found in the input stream. Only works on buffered sockets
int elua_net_set_split( int s, int schar )
{
  volatile struct elua_uip_state *pstate;

  if( eluah_get_socket_state( &s, &pstate, 0 ) == -1 )
    return 0;
  if( !pstate->buf && schar != ELUA_NET_NO_SPLIT )
    return 0;
  pstate->split = schar;
  return 1;
}

int elua_net_set_buffer( int s, unsigned bufsize )
{
  volatile struct elua_uip_state *pstate;
  int res = 1;
  char *newp, *prevp;

  if( eluah_get_socket_state( &s, &pstate, 0 ) == -1 )
    return 0;
  // Disable buffering until we make the required changes
  platform_eth_set_interrupt( PLATFORM_ETH_INT_DISABLE );
  prevp = pstate->buf;
  pstate->buf = NULL;
  platform_eth_set_interrupt( PLATFORM_ETH_INT_ENABLE );
  if( bufsize == 0 ) // no more buffers
  {
    free( prevp );
    newp = NULL;
  }
  else
  {
    if( ( newp = ( char* )realloc( prevp, bufsize ) ) == NULL )
    {
      // realloc() failed, but the old buffer (if any) is still there
      res = 0;
      newp = prevp;
    }
  }
  platform_eth_set_interrupt( PLATFORM_ETH_INT_DISABLE );
  if( ( pstate->buf = newp ) == NULL )
    pstate->split = ELUA_NET_NO_SPLIT;
  if( res )
    pstate->buf_total = bufsize;
  pstate->buf_crt = pstate->buf_ridx = pstate->buf_widx = 0;
  platform_eth_set_interrupt( PLATFORM_ETH_INT_ENABLE );
  return res;
}

// Accept a connection on the given port, return its socket id (and the IP of the remote host by side effect)
int elua_net_accept( u16 port, unsigned bufsize, unsigned timer_id, timer_data_type to_us, elua_net_ip* pfrom )
{
  timer_data_type tmrstart = 0;
  int s = -1;
  
  if( !elua_uip_configured )
    return -1;
#ifdef BUILD_CON_TCP
  if( port == ELUA_NET_TELNET_PORT )
    return -1;
#endif  
  if( ( s = elua_net_socket( ELUA_NET_SOCK_STREAM ) ) == -1 )
    return -1;
  if( bufsize )
    if( elua_net_set_buffer( s, bufsize ) == 0 )
      return -1;
  uip_conn_mark_accept( s );
  platform_eth_set_interrupt( PLATFORM_ETH_INT_DISABLE );
  uip_unlisten( htons( port ) );
  uip_listen( htons( port ) );
  platform_eth_set_interrupt( PLATFORM_ETH_INT_ENABLE );
  elua_uip_accept_request = 1;
  if( to_us != PLATFORM_TIMER_INF_TIMEOUT )
    tmrstart = platform_timer_op( timer_id, PLATFORM_TIMER_OP_START, 0 );
  while( 1 )
  {
    if( elua_uip_accept_request == 0 )
      break;
    if( !elua_uip_configured || to_us == 0 || ( to_us != PLATFORM_TIMER_INF_TIMEOUT && platform_timer_get_diff_crt( timer_id, tmrstart ) >= to_us ) )
    {
      elua_net_close( s );
      s = -1;
      break;
    }
  }  
  elua_uip_accept_request = 0;
  *pfrom = elua_uip_accept_remote;
  return s;
}

// Connect to a specified machine
int elua_net_connect( int s, elua_net_ip addr, u16 port, unsigned timer_id, timer_data_type to_us )
{
  volatile struct elua_uip_state *pstate = ( volatile struct elua_uip_state* )&( uip_conns[ s ].appstate );
  uip_ipaddr_t ipaddr;
  timer_data_type tmrstart = 0;
  
  if( !ELUA_UIP_IS_SOCK_OK( s ) )
    return -1;
  // The socket should have been reserved by a previous call to "elua_net_socket"
  if( !uip_conn_is_reserved( s ) )
    return -1;
  // Initiate the connect call  
  uip_ipaddr( ipaddr, addr.ipbytes[ 0 ], addr.ipbytes[ 1 ], addr.ipbytes[ 2 ], addr.ipbytes[ 3 ] );
  elua_prep_socket_state( pstate, NULL, 0, ELUA_NET_ERR_OK, ELUA_UIP_STATE_CONNECT );  
  if( uip_connect_socket( s, &ipaddr, htons( port ) ) == NULL )
    return -1;
  // And wait for it to finish
  if( to_us != PLATFORM_TIMER_INF_TIMEOUT )
    tmrstart = platform_timer_op( timer_id, PLATFORM_TIMER_OP_START, 0 );
  while( pstate->state != ELUA_UIP_STATE_IDLE )
  {
    if( !eluah_check_stack_state( pstate ) )
      break;
    if( to_us == 0 || ( to_us != PLATFORM_TIMER_INF_TIMEOUT && platform_timer_get_diff_crt( timer_id, tmrstart ) >= to_us ) )
    {
      elua_net_close( s );
      return -1;
    }
  }
  return pstate->res == ELUA_NET_ERR_OK ? 0 : -1;
}

// Hostname lookup (resolver)
elua_net_ip elua_net_lookup( const char* hostname )
{
  elua_net_ip res = { 0 };
  
#ifdef BUILD_DNS
  u16_t *data;
  
  if( ( data = resolv_lookup( ( char* )hostname ) ) != NULL ) // name already saved locally
    uip_ipaddr_copy( ( u16* )&res, data );
  else
  {
    // Name not saved locally, must make request
    elua_resolv_req_done = 0;
    resolv_query( ( char* )hostname );
    platform_eth_force_interrupt();
    while( elua_resolv_req_done == 0 && elua_uip_configured );
    if( elua_uip_configured )
      res = elua_resolv_ip;
  }
#endif
  return res;  
}

// UDP send to address
unsigned elua_net_sendto( int s, const void* buf, elua_net_size len, elua_net_ip remoteip, u16 port )
{
  return elua_net_send_internal( s, buf, len, remoteip, port, 1 );
}

elua_net_size eluah_net_recvfrom_common( int s, void *buf, elua_net_size maxsize, elua_net_ip *p_remote_ip, u16 *p_remote_port, unsigned timer_id, timer_data_type to_us, int with_buffer )
{
  elua_net_ip remote_ip;
  u16 remote_port;
  elua_net_size res = elua_net_recv_internal( s, buf, maxsize, timer_id, to_us, with_buffer, &remote_ip, &remote_port, 1 );

  if( p_remote_ip )
    p_remote_ip->ipaddr = remote_ip.ipaddr;
  if( p_remote_port )
    *p_remote_port = remote_port;
  return res;
}

elua_net_size elua_net_recvfrom( int s, void *buf, elua_net_size maxsize, elua_net_ip *p_remote_ip, u16 *p_remote_port, unsigned timer_id, timer_data_type to_us )
{
  return eluah_net_recvfrom_common( s, buf, maxsize, p_remote_ip, p_remote_port, timer_id, to_us, 0 );
}

elua_net_size elua_net_recvfrombuf( int s, luaL_Buffer *buf, elua_net_size maxsize, elua_net_ip *p_remote_ip, u16 *p_remote_port, unsigned timer_id, timer_data_type to_us )
{
  return eluah_net_recvfrom_common( s, buf, maxsize, p_remote_ip, p_remote_port, timer_id, to_us, 1 );
}

elua_net_ip elua_net_get_config( int what )
{
  elua_net_ip res = { 0 };
  u16 *pdns;

  if( !elua_uip_configured )
    return res;
  switch( what )
  {
    case ELUA_NET_CFG_IP:
      uip_ipaddr_copy( ( u16* )&res, uip_hostaddr );
      break;
    case ELUA_NET_CFG_NETMASK:
      uip_ipaddr_copy( ( u16* )&res, uip_netmask );
      break;
    case ELUA_NET_CFG_DNS:
      if( ( pdns = resolv_getserver() ) != NULL )
        uip_ipaddr_copy( ( u16* )&res, pdns );
      break;
    case ELUA_NET_CFG_GW:
      uip_ipaddr_copy( ( u16* )&res, uip_draddr );
      break;
  }
  return res;
}

// [TODO] these should probably go in a common_net.c source file

// Helper: read a char from a socket, wait for more data if needed
static int eluah_expect_read_char( int s, int socktype, volatile struct elua_uip_state *pstate, unsigned timer_id, timer_data_type to_us, timer_data_type tmrstart )
{
  static int leftbytes = 0;
  int c;

  if( leftbytes == 0 )
  {
    while( 1 )
    {
      if( ( leftbytes = pstate->buf_crt ) > 0 )
        break;
      if( !eluah_check_stack_state( pstate ) )
        return -1;
      if( to_us == 0 || ( to_us != PLATFORM_TIMER_INF_TIMEOUT && platform_timer_get_diff_crt( timer_id, tmrstart ) >= to_us ) )
        break;
      if( socktype == ELUA_NET_SOCK_STREAM && !uip_conn_active( s ) )
        break;
    }
    if( leftbytes == 0 )
      return -1;
  }
  c = pstate->buf[ pstate->buf_ridx ];
  leftbytes --;
  pstate->buf_ridx = pstate->buf_ridx + 1;
  if( pstate->buf_ridx == pstate->buf_total )
    pstate->buf_ridx = 0;
  platform_eth_set_interrupt( PLATFORM_ETH_INT_DISABLE );
  pstate->buf_crt --;
  platform_eth_set_interrupt( PLATFORM_ETH_INT_ENABLE );
  return c;
}

// Helper: read until a string is found, skip data or keep it in the
// given buffer according to the "buffer" argument
static int eluah_expect_internal( int s, luaL_Buffer *b, const u8 *str, unsigned len, unsigned timer_id, timer_data_type to_us )
{
  u8 *backbuf = NULL;
  int bidx = 0;
  int c, socktype;
  timer_data_type tmrstart = 0;
  volatile struct elua_uip_state *pstate;

  if( ( socktype = eluah_get_socket_state( &s, &pstate, 1 ) ) == -1 )
    return 0;
  if( !pstate->buf )
    return 0;
  if( b )
    if( ( backbuf = ( u8* )malloc( len ) ) == NULL )
      return 0;
  if( to_us != PLATFORM_TIMER_INF_TIMEOUT )
    tmrstart = platform_timer_op( timer_id, PLATFORM_TIMER_OP_START, 0 );
  while( 1 )
  {
    if( ( c = eluah_expect_read_char( s, socktype, pstate, timer_id, to_us, tmrstart ) ) == -1 ) 
      break;
    if( c == str[ bidx ] )
    {
      if( b )
        backbuf[ bidx ] = c;
      if( ++ bidx == len )
        break;
    }
    else
    {
      if( b )
      {
        luaL_addlstring( b, ( const char* )backbuf, bidx );
        luaL_addchar( b, ( char )c );
      }
      bidx = 0;
    }
  }
  if( backbuf )
    free( backbuf );
  return c == -1 ? 0 : 1;
}

int elua_net_expect( int s, const u8 *str, unsigned len, unsigned timer_id, timer_data_type to_us )
{
  return eluah_expect_internal( s, NULL, str, len, timer_id, to_us );
}

int elua_net_readto( int s, luaL_Buffer *b, const u8 *str, unsigned len, unsigned timer_id, timer_data_type to_us )
{
  return eluah_expect_internal( s, b, str, len, timer_id, to_us );
}

#endif // #ifdef BUILD_UIP

