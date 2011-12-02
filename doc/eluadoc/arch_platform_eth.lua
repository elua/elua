-- eLua platform interface - Ethernet support

data_en = 
{
  -- Title
  title = "eLua platform interface - Ethernet support",

  -- Menu name
  menu_name = "Ethernet",

  -- Overview
  overview = [[<span style="color: red;">$NOTE$: TCP/IP support is experimental in eLua. Although functional, it's quite incomplete at the moment.</span></p>
  <p>This part of the platform interface groups functions related to accessing the Ethernet interface (internal or external) of the CPU. Note that unlike the
  other parts of the platform interface this one is dedicated for TCP/IP support and thus it does not correspond directly to an eLua module, although
  the @refman_gen_net.html@net module@ is implemented with functions that rely on this part of the platform interface. Currently only
  the ^http://www.sics.se/~~adam/uip/index.php/Main_Page^uIP^ TCP/IP stack is supported by eLua.</p>
  <p>uIP is implemented in eLua using two hardware interrupts (that should be available on your platform): the Ethernet receive interrupt (to handle
  incoming packets) and a timer interrupt (timers are used internally by uIP). However, the uIP main loop is only called from the Ethernet interrupt handler
  in eLua, so in order to acknowledge the timer interrupt (as well as to provide some optimizations) a function that "forces" an Ethernet interrupt
  must also be provided by the platform interface (see @#platform_eth_force_interrupt@here@ for details).</p>
  <p>To put everything together, part of the Ethernet platform interface for the $lm3s$ platform is given below:</p>
  ~u32 platform_eth_get_elapsed_time()
  {
    if( eth_timer_fired )
    {
      eth_timer_fired = 0;
      return SYSTICKMS;
    }
    else
      return 0;
  }

  void SysTickIntHandler()
  {
    // Handle virtual timers
    cmn_virtual_timer_cb();

    // Indicate that a SysTick interrupt has occurred.
    eth_timer_fired = 1;

    // Generate a fake Ethernet interrupt.  This will perform the actual work
    // of incrementing the timers and taking the appropriate actions.
    <b>platform_eth_force_interrupt();</b>
  }

  void EthernetIntHandler()
  {
    u32 temp;

    // Read and Clear the interrupt.
    temp = EthernetIntStatus( ETH_BASE, false );
    EthernetIntClear( ETH_BASE, temp );

    // Call the UIP main loop
    <b>elua_uip_mainloop();</b>
  }~<p>]],

  -- Functions
  funcs = 
  {
    { sig = "void #platform_eth_send_packet#( const void* src, u32 size )",
      desc = "Sends an Ethernet packet to the network",
      args = 
      {
        "$src$ - start address of the Ethernet packet",
        "$size$ - size of the Ethernet packet"
      },
    },

    { sig = "u32 #platform_eth_get_packet_nb#( void* buf, u32 maxlen );",
      desc = "Non-blocking read of an Ethernet packet from the network",
      args = 
      {
        "$buf$ - start address of the receive buffer",
        "$maxlen$ - maximum length of the Ethernet packet",
      },
      ret = "the size of the read packet or 0 if no packet is available"
    },

    { sig = "void #platform_eth_force_interrupt#();",
      desc = "Force the Ethernet interrupt on the platform (see @#overview@overview@ above for details)",
    },

    { sig = "u32 #platform_eth_get_elapsed_time#();",
      desc = [[Get the elapsed time (in ms) since the last invocation of the uIP main loop ($elua_uip_mainloop$, from which this function is called). See @#overview@overview@ for a possible 
  implementation of this function).]], 
      ret = 
      { 
        "0 if the uIP loop was called because of Ethernet activity, not because a timer expired",
        "the Ethernet timer period in ms (which indicates timer activity)"
      },
    }
  }
}


data_pt = data_en
