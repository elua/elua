// UIP "helper" for eLua
// Implements the eLua specific UIP application

#ifndef __ELUA_UIP_H__
#define __ELUA_UIP_H__

#include "type.h"
#include "elua_net.h"

// eLua UIP application states
enum
{
  ELUA_UIP_STATE_IDLE = 0,
  ELUA_UIP_STATE_SEND,
  ELUA_UIP_STATE_RECV,
  ELUA_UIP_STATE_CONNECT,
  ELUA_UIP_STATE_CLOSE,
  ELUA_UIP_STATE_CLOSE_ACK,
  ELUA_UIP_STATE_RETRY
};

#define ELUA_UIP_UDP_FIRST_ID      16384
#define ELUA_UIP_IS_UDP( s )       ( ( s ) >= ELUA_UIP_UDP_FIRST_ID )
#define ELUA_UIP_TO_UDP( s )       ( ( s ) >= 0 ? ( s ) + ELUA_UIP_UDP_FIRST_ID : ( s ) )
#define ELUA_UIP_FROM_UDP( s )     ( ( s ) >= ELUA_UIP_UDP_FIRST_ID ? ( s ) - ELUA_UIP_UDP_FIRST_ID : -1 ) 

// eLua UIP state
struct elua_uip_state
{
  u8                state, res;
  char              *ptr, *buf; 
  elua_net_size     len;
  s16               split;
  elua_net_size     buf_total, buf_crt, buf_ridx, buf_widx;
  p_elua_net_recv_cb  recv_cb;
};

struct uip_eth_addr;

// Helper functions
void elua_uip_appcall();
void elua_uip_udp_appcall();
void elua_uip_init( const struct uip_eth_addr* paddr );
void elua_uip_mainloop();

#endif
