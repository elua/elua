// UIP support functions

#include "build.h"
#ifdef BUILD_UIP

#include "hw_types.h"
#include "hw_ints.h"
#include "hw_ethernet.h"
#include "hw_memmap.h"
#include "ethernet.h"
#include "type.h"
#include "hw_nvic.h"
#include "uip.h"
#include "uip_arp.h"

// Interrupt flags
#define INT_TIMER     1
#define INT_ETH_RX    2
static u8 int_flags;

// Timers
static u32 periodic_timer;
static u32 arp_timer;

// Macro for accessing the Ethernet header information in the buffer.
#define BUF                     ((struct uip_eth_hdr *)&uip_buf[0])

// UIP Timers (in ms)
#define UIP_PERIODIC_TIMER_MS   500
#define UIP_ARP_TIMER_MS        10000

// This gets called on both Ethernet RX interrupts and timer requests,
// but it's called only from the Ethernet interrupt handler
static void uip_support_func()
{
  long packet_len;
  u32 temp;
  
  // If SysTick, Clear the SysTick interrupt flag and increment the
  // timers.
  if( int_flags & INT_TIMER )
  {
    int_flags &= ( u8 )~INT_TIMER;
    periodic_timer += SYSTICKMS;
    arp_timer += SYSTICKMS;
  }

  // Check for an RX packet and read it
  packet_len = EthernetPacketGetNonBlocking(ETH_BASE, uip_buf, sizeof(uip_buf));
  if(packet_len > 0)
  {
    // Set uip_len for uIP stack usage.
    uip_len = (unsigned short)packet_len;

    // Clear the RX Packet event and renable RX Packet interrupts.
    if( int_flags & INT_ETH_RX )
    {
      int_flags &= ( u8 )~INT_ETH_RX;
      EthernetIntEnable(ETH_BASE, ETH_INT_RX);
    }

    // Process incoming IP packets here.
    if(BUF->type == htons(UIP_ETHTYPE_IP))
    {
      uip_arp_ipin();
      uip_input();

      // If the above function invocation resulted in data that
      // should be sent out on the network, the global variable
      // uip_len is set to a value > 0.
      if(uip_len > 0)
      {
        uip_arp_out();
        EthernetPacketPut(ETH_BASE, uip_buf, uip_len);
        uip_len = 0;
      }
    }

    // Process incoming ARP packets here.
    else if(BUF->type == htons(UIP_ETHTYPE_ARP))
    {
      uip_arp_arpin();

      // If the above function invocation resulted in data that
      // should be sent out on the network, the global variable
      // uip_len is set to a value > 0.
      if(uip_len > 0)
      {
          EthernetPacketPut(ETH_BASE, uip_buf, uip_len);
          uip_len = 0;
      }
    }
  }  
  
  // Process TCP/IP Periodic Timer here.
  if(periodic_timer >= UIP_PERIODIC_TIMER_MS)
  {
    periodic_timer = 0;
    for( temp = 0; temp < UIP_CONNS; temp ++ )
    {
      uip_periodic(temp);

      // If the above function invocation resulted in data that
      // should be sent out on the network, the global variable
      // uip_len is set to a value > 0.
      if(uip_len > 0)
      {
        uip_arp_out();
        EthernetPacketPut(ETH_BASE, uip_buf, uip_len);
        uip_len = 0;
      }
    }

#if UIP_UDP
    for( temp = 0; temp < UIP_UDP_CONNS; temp ++ )
    {
      uip_udp_periodic( temp );

      // If the above function invocation resulted in data that
      // should be sent out on the network, the global variable
      // uip_len is set to a value > 0.
      if(uip_len > 0)
      {
        uip_arp_out();
        EthernetPacketPut(ETH_BASE, uip_buf, uip_len);
        uip_len = 0;
      }
    }
#endif // UIP_UDP
  }  
  
  // Process ARP Timer here.
  if(arp_timer >= UIP_ARP_TIMER_MS)
  {
    arp_timer = 0;
    uip_arp_timer();
  }  
}

void SysTickIntHandler()
{
  // Indicate that a SysTick interrupt has occurred.
  int_flags |= INT_TIMER;

  // Generate a fake Ethernet interrupt.  This will perform the actual work
  // of incrementing the timers and taking the appropriate actions.
  HWREG(NVIC_SW_TRIG) |= INT_ETH - 16;
}

void EthernetIntHandler()
{
  u32 temp;
  
  // Read and Clear the interrupt.
  temp = EthernetIntStatus(ETH_BASE, false);
  EthernetIntClear(ETH_BASE, temp);

  // Check to see if an RX Interrupt has occured.
  if(temp & ETH_INT_RX)
  {
    // Indicate that a packet has been received.
    int_flags |= INT_ETH_RX;

    // Disable Ethernet RX Interrupt.
    EthernetIntDisable(ETH_BASE, ETH_INT_RX);
  }
  
  uip_support_func();
}

#else // #ifdef BUILD_UIP

// "Dummy" (empty) handler functions
void EthernetIntHandler()
{
}

void SysTickIntHandler()
{
}

#endif // #ifdef BUILD_UIP
