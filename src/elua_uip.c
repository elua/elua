#include "platform_conf.h"
#ifdef BUILD_UIP

// UIP "helper" for eLua
// Implements the eLua specific UIP application

#include "elua_uip.h"
#include "elua_net.h"
#include "uip.h"
#include "uip_arp.h"
#include "platform.h"
#include "utils.h"
#include "uip-split.h"
#include "dhcpc.h"
#include "resolv.h"
#include <string.h>

// UIP send buffer
extern void* uip_sappdata;

// Global "configured" flag
static volatile u8 elua_uip_configured;

// *****************************************************************************
// Platform independenet eLua UIP "main loop" implementation

// Timers
static u32 periodic_timer, arp_timer;

// Macro for accessing the Ethernet header information in the buffer.
#define BUF                     ((struct uip_eth_hdr *)&uip_buf[0])

// UIP Timers (in ms)
#define UIP_PERIODIC_TIMER_MS   500
#define UIP_ARP_TIMER_MS        10000

#define IP_TCP_HEADER_LENGTH 40
#define TOTAL_HEADER_LENGTH (IP_TCP_HEADER_LENGTH+UIP_LLH_LEN)

static void device_driver_send()
{
  platform_eth_send_packet( uip_buf, uip_len );
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
  while( ( packet_len = platform_eth_get_packet_nb( uip_buf, sizeof( uip_buf ) ) ) > 0 )
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
    uip_sethostaddr( s->ipaddr );
    uip_setnetmask( s->netmask );
    uip_setdraddr( s->default_router );
    resolv_conf( ( u16_t* )s->dnsaddr );
    elua_uip_configured = 1;
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
  {
    elua_resolv_ip.ipwords[ 0 ] = ipaddr[ 0 ];
    elua_resolv_ip.ipwords[ 1 ] = ipaddr[ 1 ];
  }
  elua_resolv_req_done = 1;
}
#endif

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
    uip_stop();
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

#endif // #ifdef BUILD_CON_TCP

// *****************************************************************************
// eLua UIP application (used to implement the eLua TCP/IP services)



typedef struct {
  u8 accept_request; // 0=accepted/not used 1=pending
  int sock;   // socket number when accepted
  elua_net_ip remote; // remote IP
  int port; // local port bound to

} elua_uip_accept_pending_t;

volatile static elua_uip_accept_pending_t elua_uip_accept_pending[UIP_CONF_MAX_CONNECTIONS];

volatile static struct uip_conn *elua_net_connecting=NULL;


void elua_uip_appcall()
{
  volatile struct elua_uip_state *s;
  elua_net_size temp;
  int sockno;

  // If uIP is not yet configured (DHCP response not received), do nothing
  if( !elua_uip_configured )
    return;

  s = ( struct elua_uip_state* )&( uip_conn->appstate );
  // Need to find the actual socket location, since UIP doesn't provide this ...
  for( temp = 0; temp < UIP_CONNS; temp ++ )
    if( uip_conns + temp == uip_conn )
      break;
  sockno = ( int )temp;

  if( uip_connected() )
  {
    // check if we are currenty in a conncet call and the socket
    // in the connection is the right one...
    if  ( s->state == ELUA_UIP_STATE_CONNECT  && uip_conn==elua_net_connecting ) {
      s->state = ELUA_UIP_STATE_IDLE;
    }

#ifdef BUILD_CON_TCP
    else if( uip_conn->lport == HTONS( ELUA_NET_TELNET_PORT ) ) // special case: telnet server
    {
      if( elua_uip_telnet_socket != -1 )
      {
        uip_close();
        return;
      }
      else
        elua_uip_telnet_socket = sockno;
    }

#endif
   else {
     // If we have no active connect call, the event can only come
     // from a connection to a listening port. In this case add it
     // to the pending request array, so it can later be taken by a
     // call to elua_accept
     int i;
     BOOL found=FALSE;
     for( i=0;i<UIP_CONF_MAX_CONNECTIONS && !found;i++ )
     {
        if ( elua_uip_accept_pending[i].accept_request!=1 ) {// free slot
          elua_uip_accept_pending[i].sock=sockno;
          elua_uip_accept_pending[i].remote.ipwords[0]=uip_conn->ripaddr[ 0 ];
          elua_uip_accept_pending[i].remote.ipwords[1]=uip_conn->ripaddr[ 1 ];
          elua_uip_accept_pending[i].accept_request=1;
          elua_uip_accept_pending[i].port= HTONS( uip_conn->lport );
          found=TRUE;
        }
     }
     if ( !found ) { // no free slot
       uip_close();
       return;
     }
   }

    uip_stop();
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
    s->res = uip_aborted() ? ELUA_NET_ERR_ABORTED : ( uip_timedout() ? ELUA_NET_ERR_TIMEDOUT : ELUA_NET_ERR_CLOSED );
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
      elua_net_size minlen = UMIN( s->len, uip_mss() );
      s->len -= minlen;
      s->ptr += minlen;
      if( s->len == 0 )
        s->state = ELUA_UIP_STATE_IDLE;
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
        uip_send( s->ptr, UMIN( s->len, uip_mss() ) );
    }
    return;
  }

  // Handle close
  if( s->state == ELUA_UIP_STATE_CLOSE )
  {
    uip_close();
    s->state = ELUA_UIP_STATE_IDLE;
    return;
  }

  // Handle data receive
  if( uip_newdata() )
  {
    if( s->state == ELUA_UIP_STATE_RECV_2 )
    {
#ifdef BUILD_CON_TCP
      if( sockno == elua_uip_telnet_socket )
      {
        elua_uip_telnet_handle_input( s );
        return;
      }
#endif
      int lastfound = 0;

      // Check end of transmission
      if( uip_datalen() < UIP_RECEIVE_WINDOW )
        lastfound = 1;
      // Check overflow
      if( s->len < uip_datalen() )
      {
        s->res = ELUA_NET_ERR_OVERFLOW;
        temp = s->len;
      }
      else
        temp = uip_datalen();

      if( s->readto != ELUA_NET_NO_LASTCHAR )
      {
        char *tptr = ( char* )uip_appdata;
        char *last = ( char* )uip_appdata + temp - 1;
        luaL_Buffer *pbuf = ( luaL_Buffer* )s->ptr;
        char* dest = ( char* )s->ptr;

        while( tptr <= last )
        {
          if( *tptr == s->readto )
          {
            lastfound = 1;
            break;
          }
          if( *tptr != '\r' )
          {
            if( s->res )
              luaL_addchar( pbuf, *tptr );
            else
              *dest ++ = *tptr;
            s->len --;
          }
          tptr ++;
        }
      }
      else
      {
        if( s->res )
          luaL_addlstring( ( luaL_Buffer* )s->ptr, ( const char* )uip_appdata, temp );
        else
          memcpy( ( char* )s->ptr, ( const char* )uip_appdata, temp );
        s->len -= temp;
      }

      // Do we need to read another packet?
      if( s->len == 0 || lastfound )
      {
        uip_stop();
        s->res = ELUA_NET_ERR_OK;
        s->state = ELUA_UIP_STATE_IDLE;
      }
    }
    else
      uip_stop();
  }
}

static void elua_uip_conf_static()
{
  uip_ipaddr_t ipaddr;
  uip_ipaddr( ipaddr, ELUA_CONF_IPADDR0, ELUA_CONF_IPADDR1, ELUA_CONF_IPADDR2, ELUA_CONF_IPADDR3 );
  uip_sethostaddr( ipaddr );
  uip_ipaddr( ipaddr, ELUA_CONF_NETMASK0, ELUA_CONF_NETMASK1, ELUA_CONF_NETMASK2, ELUA_CONF_NETMASK3 );
  uip_setnetmask( ipaddr );
  uip_ipaddr( ipaddr, ELUA_CONF_DEFGW0, ELUA_CONF_DEFGW1, ELUA_CONF_DEFGW2, ELUA_CONF_DEFGW3 );
  uip_setdraddr( ipaddr );
  uip_ipaddr( ipaddr, ELUA_CONF_DNS0, ELUA_CONF_DNS1, ELUA_CONF_DNS2, ELUA_CONF_DNS3 );
  resolv_conf( ipaddr );
  elua_uip_configured = 1;
}

// Init application
void elua_uip_init( const struct uip_eth_addr *paddr )
{
  // Set hardware address
  uip_setethaddr( (*paddr) );

  // Initialize the uIP TCP/IP stack.
  uip_init();
  uip_arp_init();

  // Initalize the pending accept array
  int i;
  for( i=0;i<UIP_CONF_MAX_CONNECTIONS;i++ ) {
    elua_uip_accept_pending[i].accept_request=0;
  }


#ifdef BUILD_DHCPC
  dhcpc_init( paddr->addr, sizeof( *paddr ) );
  dhcpc_request();
#else
  elua_uip_conf_static();
#endif

  resolv_init();

#ifdef BUILD_CON_TCP
  uip_listen( HTONS( ELUA_NET_TELNET_PORT ) );
#endif
}

// *****************************************************************************
// eLua UIP UDP application (used for the DHCP client and the DNS resolver)

void elua_uip_udp_appcall()
{
  resolv_appcall();
  dhcpc_appcall();
}

// *****************************************************************************
// eLua TCP/IP services (from elua_net.h)

#define ELUA_UIP_IS_SOCK_OK( sock ) ( elua_uip_configured && sock >= 0 && sock < UIP_CONNS )

static void elua_prep_socket_state( volatile struct elua_uip_state *pstate, void* buf, elua_net_size len, s16 readto, u8 res, u8 state )
{
  pstate->ptr = ( char* )buf;
  pstate->len = len;
  pstate->res = res;
  pstate->readto = readto;
  pstate->state = state;
}

int elua_net_socket( int type )
{
  int i;
  struct uip_conn* pconn;
  int old_status;

  // [TODO] add UDP support at some point.
  if( type == ELUA_NET_SOCK_DGRAM )
    return -1;

  old_status = platform_cpu_set_global_interrupts( PLATFORM_CPU_DISABLE );
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
  platform_cpu_set_global_interrupts( old_status );
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
  elua_prep_socket_state( pstate, ( void* )buf, len, ELUA_NET_NO_LASTCHAR, ELUA_NET_ERR_OK, ELUA_UIP_STATE_SEND );
  platform_eth_force_interrupt();
  while( pstate->state != ELUA_UIP_STATE_IDLE );
  return len - pstate->len;
}

// Internal "read" function
static elua_net_size elua_net_recv_internal( int s, void* buf, elua_net_size maxsize, s16 readto, unsigned timer_id, timer_data_type to_us, int with_buffer )
{
  volatile struct elua_uip_state *pstate = ( volatile struct elua_uip_state* )&( uip_conns[ s ].appstate );
  timer_data_type tmrstart = 0;
  int old_status;

  if( !ELUA_UIP_IS_SOCK_OK( s ) || !uip_conn_active( s ) )
    return -1;
  if( maxsize == 0 )
    return 0;
  elua_prep_socket_state( pstate, buf, maxsize, readto, with_buffer, ELUA_UIP_STATE_RECV );
  if( to_us > 0 )
    tmrstart = platform_timer_start( timer_id );
  while( 1 )
  {
    if( pstate->state == ELUA_UIP_STATE_IDLE )
      break;
    if( to_us > 0 && platform_timer_get_diff_crt( timer_id, tmrstart ) >= to_us )
    {
      old_status = platform_cpu_set_global_interrupts( PLATFORM_CPU_DISABLE );
      if( pstate->state != ELUA_UIP_STATE_IDLE )
      {
        pstate->res = ELUA_NET_ERR_TIMEDOUT;
        pstate->state = ELUA_UIP_STATE_IDLE;
      }
      platform_cpu_set_global_interrupts( old_status );
      break;
    }
  }
  return maxsize - pstate->len;
}

// Receive data in buf, upto "maxsize" bytes, or upto the 'readto' character if it's not -1
elua_net_size elua_net_recv( int s, void* buf, elua_net_size maxsize, s16 readto, unsigned timer_id, timer_data_type to_us )
{
  return elua_net_recv_internal( s, buf, maxsize, readto, timer_id, to_us, 0 );
}

// Same thing, but with a Lua buffer as argument
elua_net_size elua_net_recvbuf( int s, luaL_Buffer* buf, elua_net_size maxsize, s16 readto, unsigned timer_id, timer_data_type to_us )
{
  return elua_net_recv_internal( s, buf, maxsize, readto, timer_id, to_us, 1 );
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
  elua_prep_socket_state( pstate, NULL, 0, ELUA_NET_NO_LASTCHAR, ELUA_NET_ERR_OK, ELUA_UIP_STATE_CLOSE );
  platform_eth_force_interrupt();
  while( pstate->state != ELUA_UIP_STATE_IDLE );
  return pstate->res == ELUA_NET_ERR_OK ? 0 : -1;
}

// Get last error on specific socket
int elua_net_get_last_err( int s )
{
  volatile struct elua_uip_state *pstate = ( volatile struct elua_uip_state* )&( uip_conns[ s ].appstate );

  if( !ELUA_UIP_IS_SOCK_OK( s ) )
    return -1;
  return pstate->res;
}


// New TH: listen/unlistens a port
int elua_listen(u16 port,BOOL flisten)
{
int old_status;

 if( !elua_uip_configured )
    return -1;
#ifdef BUILD_CON_TCP
  if( port == ELUA_NET_TELNET_PORT )
    return -1;
#endif

  old_status = platform_cpu_set_global_interrupts( PLATFORM_CPU_DISABLE );

  uip_unlisten( htons( port ) );
  if (flisten)
     uip_listen( htons( port ) );

  platform_cpu_set_global_interrupts( old_status );

  return 0;

}


static int elua_net_find_pending( u16 port )
{
int i;


  for( i=0;i<UIP_CONF_MAX_CONNECTIONS;i++ )
  {
    if ( elua_uip_accept_pending[i].accept_request==1
        && elua_uip_accept_pending[i].port==port )
      return i;
  }
  return -1;
}


// Accept a connection on the given port, return its socket id (and the IP of the remote host by side effect)
// TH: Changed behaviour: Does no own listen to the port, this has to be done before with a call to elua_listen
// accept will just look for pending connections to the port and will return the socket of the first one
// so it can also be called with a timeout of 0 and return in a polled loop
int elua_accept( u16 port, unsigned timer_id, timer_data_type to_us, elua_net_ip* pfrom )
{
  timer_data_type tmrstart = 0;
  int old_status;

  int i;

  if( !elua_uip_configured )
    return -1;


  if( to_us > 0 )
    tmrstart = platform_timer_start( timer_id );
  while( 1 )
  {
    i=elua_net_find_pending( port );
    if( i >= 0 ) {
      *pfrom = elua_uip_accept_pending[i].remote;
      old_status=platform_cpu_set_global_interrupts( PLATFORM_CPU_DISABLE );
      elua_uip_accept_pending[i].accept_request=0;
      platform_cpu_set_global_interrupts( old_status );
      return elua_uip_accept_pending[i].sock;
    }

    if( to_us == 0 || platform_timer_get_diff_crt( timer_id, tmrstart ) >= to_us )
    {
      return -1;
    }
  }
}



// Connect to a specified machine
int elua_net_connect( int s, elua_net_ip addr, u16 port )
{
  volatile struct elua_uip_state *pstate = ( volatile struct elua_uip_state* )&( uip_conns[ s ].appstate );
  uip_ipaddr_t ipaddr;

  if( !ELUA_UIP_IS_SOCK_OK( s ) )
    return -1;
  // The socket should have been reserved by a previous call to "elua_net_socket"
  if( !uip_conn_is_reserved( s ) )
    return -1;
  // Initiate the connect call
  uip_ipaddr( ipaddr, addr.ipbytes[ 0 ], addr.ipbytes[ 1 ], addr.ipbytes[ 2 ], addr.ipbytes[ 3 ] );
  elua_prep_socket_state( pstate, NULL, 0, ELUA_NET_NO_LASTCHAR, ELUA_NET_ERR_OK, ELUA_UIP_STATE_CONNECT );
  elua_net_connecting=uip_connect_socket( s, &ipaddr, htons( port ) ) ;
  if( elua_net_connecting == NULL )
    return -1;
  // And wait for it to finish
  while( pstate->state != ELUA_UIP_STATE_IDLE );
  elua_net_connecting=NULL;
  return pstate->res == ELUA_NET_ERR_OK ? 0 : -1;
}

// Hostname lookup (resolver)
elua_net_ip elua_net_lookup( const char* hostname )
{
  elua_net_ip res;

  res.ipaddr = 0;
#ifdef BUILD_DNS
  u16_t *data;

  if( ( data = resolv_lookup( ( char* )hostname ) ) != NULL )
  {
    // Name already saved locally
    res.ipwords[ 0 ] = data[ 0 ];
    res.ipwords[ 1 ] = data[ 1 ];
  }
  else
  {
    // Name not saved locally, must make request
    elua_resolv_req_done = 0;
    resolv_query( ( char* )hostname );
    platform_eth_force_interrupt();
    while( elua_resolv_req_done == 0 );
    res = elua_resolv_ip;
  }
#endif
  return res;
}

#endif // #ifdef BUILD_UIP
