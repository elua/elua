// UIP "helper" for eLua
// Implements the eLua specific UIP application

#ifndef __ELUA_UIP_H__
#define __ELUA_UIP_H__

// eLua UIP state
struct elua_uip_state
{
  int dummy;
};

void elua_uip_appcall();
void elua_uip_init();

#endif
