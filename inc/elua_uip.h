// UIP "helper" for eLua
// Implements the eLua specific UIP application

#ifndef __ELUA_UIP_H__
#define __ELUA_UIP_H__

#include "elua_net.h"

// eLua UIP application states
enum
{
  ELUA_UIP_STATE_IDLE = 0,
  ELUA_UIP_STATE_SEND,
  ELUA_UIP_STATE_RECV,
  ELUA_UIP_STATE_RECV_2,
  ELUA_UIP_STATE_CONNECT,
  ELUA_UIP_STATE_CLOSE
};

// eLua UIP state
struct elua_uip_state
{
  u8                state, res;
  char*             ptr; 
  elua_net_size     len;
  s16               readto;
};

struct uip_eth_addr;

// Helper functions
void elua_uip_appcall();
void elua_uip_udp_appcall();
void elua_uip_init( const struct uip_eth_addr* paddr );
void elua_uip_mainloop();

#endif
