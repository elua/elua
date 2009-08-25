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
  be available by using $tmr.VIRT0$ to $tmr.VIRT3$ as timer IDs.</p>
  <p>All "time units" (delays, differences in time) in this module, as well as in other parts of eLua (timeouts) are expressed in microseconds. However,
  please keep in mind that the actual timer resolution depends on many factors. For example, it's very likely that the @#tmr.delay@tmr.delay@ function won't
  be able to delay for the exact amount you specify (in us), as the real delay depends ona number of variables, most notably the base clock of the timer 
  and the size of the timer counter register (32 bits on some platforms, 16 bits on most platforms, other values are less common). To ensure that the delay 
  you're requesting is achievable, use @#tmr.getmindelay@tmr.getmindelay@ and @#tmr.getmaxdelay@tmr.getmaxdelay@ to obtain the maximum and the minimum 
  achievable wait times on your timer, respectively. Even if your delay is within these limits, the $precision$ of this function still varies a lot, 
  mainly as a function of the timer base clock.]],

  -- Functions
  funcs = 
  {
    { sig = "#tmr.delay#( id, period )",
      desc = "Waits for the specified period, then returns.",
      args = 
      {
        "$period$ - the timer ID.",
        "$period$ - how long to wait (in us)."
      }
    },

    { sig = "counter = #tmr.read#( id )",
      desc= "Reads the timer counter register.",
      args = "$id$ - the timer ID.",
      ret = "The value of the timer counter register."
    },

    { sig = "counter = #tmr.start#( id )",
      desc = "Starts the specified timer.",
      args = "$id$ - the timer ID.",
      ret = "The value of the timer counter register when the timer started.",
    },

    { sig = "delta = #tmr.gettimediff#( id, counter1, counter2 )",
      desc = "Computes the time difference between two timer counter values (you can get counter values by calling @#tmr.read@tmr.read@ or @#tmr.start@tmr.start@).",
      args = 
      {
        "$id$ - the timer ID.",
        "$counter1$ - the first counter value.",
        "$counter2$ - the second counter value.",
      },
      ret = "The time difference (in us)."
    },

    { sig = "mindelay = #tmr.getmindelay#( id )",
      desc = "Get the minimum achieavable delay on the specified timer.",
      args = "$id$ - the timer ID.",
      ret = "The minimum achievable delay on the specified timer (in us)."
    },

    { sig = "maxdelay = #tmr.getmaxdelay#( id )",
      desc = "Get the maximum achieavable delay on the specified timer.",
      args = "$id$ - the timer ID.",
      ret = "The maximum achievable delay on the specified timer (in us)."
    },

    { sig = "clock = #tmr.setclock#( id, clock )",
      desc = "Set the timer clock (the clock used to increment the timer counter register).",
      args = 
      {
        "$id$ - the timer ID.",
        "$clock$ - the timer clock (in Hz)."
      },
       ret = [[The actual clock set on the timer (in Hz). Depending on the hardware, this might have a different value than the $clock$ argument. 
$NOTE:$ this function does not work with virtual timers.]]       
    },

    { sig = "clock = #tmr.getclock#( id )",
      desc = "Get the timer clock (the clock used to increment the timer counter register).",
      args = "$id$ - the timer ID.",
      ret = "The timer clock (in Hz)."
    }
   
  }

}

