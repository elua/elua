-- eLua reference manual - tmr module

data_en = 
{

  -- Title
  title = "eLua reference manual - tmr module",

  -- Menu name
  menu_name = "tmr",

  -- Overview
  overview = [[This module contains functions for accessing the hardware timers of the eLua CPU. In addition, if virtual timers are enabled 
  (see @arch_platform_timers.html#virtual_timers@here@ and @building.html@here@ for details), they can be used just like the "regular" (hardware)
  timers with a single exception: you can't set the clock of a virtual timer (using @#tmr.setclock@tmr.setclock@). To use virtual timers with this
  module, specify $tmr.VIRTx$ as the timer ID instead of a number. For example, if the eLua image was configured to support 4 virtual timers, they will
  be available by using $tmr.VIRT0$ to $tmr.VIRT3$ as timer IDs. The @arch_platform_timers.html#the_system_timer@system timer@ can also be used with
  any of these functions by omitting the timer ID or specifying it as $tmr.SYS_TIMER$.</p>
  <p>All "time units" (delays, differences in time) in this module, as well as in other parts of eLua (timeouts) are expressed in microseconds. However,
  please keep in mind that the actual timer resolution depends on many factors. For example, it's very likely that the @#tmr.delay@tmr.delay@ function won't
  be able to delay for the exact amount you specify (in us), as the real delay depends on a number of variables, most notably the base clock of the timer 
  and the size of the timer counter register (32 bits on some platforms, 16 bits on most platforms, other values are less common). To ensure that the delay 
  you're requesting is achievable, use @#tmr.getmindelay@tmr.getmindelay@ and @#tmr.getmaxdelay@tmr.getmaxdelay@ to obtain the maximum and the minimum 
  achievable wait times on your timer, respectively. Even if your delay is within these limits, the $precision$ of this function still varies a lot, 
  mainly as a function of the timer base clock. Using the @arch_platform_timers.html#the_system_timer@system timer@ is highly encouraged if it is
  available on the platform as it can eliminate the forementioned problems.]],

  -- Functions
  funcs = 
  {
    { sig = "#tmr.delay#( period, [id] )",
      desc = "Waits for the specified period, then returns.",
      args = 
      {
        "$period$ - how long to wait (in us).",
        "$id (optional)$ - the timer ID. If not specified it defaults to the @arch_platform_timers.html#the_system_timer@system timer@.",
      }
    },

    { sig = "counter = #tmr.read#( [id] )",
      desc= "Reads the timer counter register.",
      args = "$id (optional)$ - the timer ID. If not specified it defaults to the @arch_platform_timers.html#the_system_timer@system timer@.",
      ret = "The value of the timer counter register."
    },

    { sig = "counter = #tmr.start#( [id] )",
      desc = "Starts the specified timer.",
      args = "$id (optional)$ - the timer ID. If not specified it defaults to the @arch_platform_timers.html#the_system_timer@system timer@.",
      ret = "The value of the timer counter register when the timer started.",
    },

    { sig = "delta = #tmr.gettimediff#( end, start, [id] )",
      desc = [[Computes the time difference between two timer counter values (obtained by calling @#tmr.read@tmr.read@ or @#tmr.start@tmr.start@). <span class="warning">NOTE</span>: the order 
of $end$ and $start$ is important. $end$ must correspond to a moment in time which came after $start$. The function knows how to deal with $a single$ timer overflow condition ($end$ is less than $start$); if the timer overflowed 2 or more times between $start$ and $end$ the result of this function will be incorrect.]],
      args = 
      {
        "$end$ - the final counter value.",
        "$start$ - the initial counter value.",
        "$id (optional)$ - the timer ID. If not specified it defaults to the @arch_platform_timers.html#the_system_timer@system timer@.",
      },
      ret = "The time difference (in us)."
    },
    
    { sig = "delta = #tmr.getdiffnow#( start, [id] )",
      desc = [[Computes the time difference between a counter value from the past (obtained by calling @#tmr.read@tmr.read@ or @#tmr.start@tmr.start@) and the counter value corresponding to the current time.]],
      args = 
      {
        "$start$ - the initial counter value.",
        "$id (optional)$ - the timer ID. If not specified it defaults to the @arch_platform_timers.html#the_system_timer@system timer@.",
      },
      ret = "The time difference (in us)."
    },


    { sig = "mindelay = #tmr.getmindelay#( [id] )",
      desc = "Get the minimum achievable delay on the specified timer.",
      args = "$id (optional)$ - the timer ID. If not specified it defaults to the @arch_platform_timers.html#the_system_timer@system timer@.",
      ret = "The minimum achievable delay on the specified timer (in us)."
    },

    { sig = "maxdelay = #tmr.getmaxdelay#( [id] )",
      desc = "Get the maximum achievable delay on the specified timer.",
      args = "$id (optional)$ - the timer ID. If not specified it defaults to the @arch_platform_timers.html#the_system_timer@system timer@.",
      ret = "The maximum achievable delay on the specified timer (in us)."
    },

    { sig = "clock = #tmr.setclock#( clock, [id] )",
      desc = "Set the timer clock (the clock used to increment the timer counter register).",
      args = 
      {
        "$clock$ - the timer clock (in Hz). ",
        "$id (optional)$ - the timer ID. If not specified it defaults to the @arch_platform_timers.html#the_system_timer@system timer@.",
      },
       ret = [[The actual clock set on the timer (in Hz). Depending on the hardware, this might have a different value than the $clock$ argument. 
$NOTE:$ this function does not work with virtual timers or with the system timer.]]       
    },

    { sig = "clock = #tmr.getclock#( [id] )",
      desc = "Get the timer clock (the clock used to increment the timer counter register).",
      args = "$id (optional)$ - the timer ID. If not specified it defaults to the @arch_platform_timers.html#the_system_timer@system timer@.",
      ret = "The timer clock (in Hz)."
    },

    { sig = "#tmr.set_match_int#( period, type, [id] )",
      desc = "Setup the timer match interrupt. Only available if interrupt support is enabled, check @inthandlers.html@here@ for details.",
      args = 
      {
        "$period$ - the interrupt period in microseconds. Setting this to 0 disabled the timer match interrupt.",
        "$type$ - $tmr.INT_ONESHOT$ to generate a single interrupt after *period* microseconds, or $tmr.INT_CYCLIC$ to generate interrupts every $period$ microseconds.",
        [[$id (optional)$ - the timer ID. If not specified it defaults to the @arch_platform_timers.html#the_system_timer@system timer@ (but note that this happens only for consistency, as the system timer can't generate interrupts).]],
      }
    }

  }
}

