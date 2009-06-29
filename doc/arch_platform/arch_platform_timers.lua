-- eLua platform interface - timers

-- Title
title = "eLua platform interface - timers"

-- Overview
overview = [[This part of the platform interface groups functions related to the timers of the MCU. It also makes provisions for using $virtual timers$ on any platform, see @#virtual@this section@
for details. Keep in mind that in the following paragraphs a $timer id$ can reffer to both a hardware timer or a virtual timer.]]

-- Data structures, constants and types
structures = 
{
  { text = "typedef u32 timer_data_type;",
    desc = "This defines the data type used to specify delays and time intervals (which are always specifide in $microseconds$)."
  },

  { text = [[// Timer operations
enum
{
  PLATFORM_TIMER_OP_START,
  PLATFORM_TIMER_OP_READ,
  PLATFORM_TIMER_OP_SET_CLOCK,
  PLATFORM_TIMER_OP_GET_CLOCK,
  PLATFORM_TIMER_OP_GET_MAX_DELAY,
  PLATFORM_TIMER_OP_GET_MIN_DELAY
};]], 
    link = "opval",
    desc = "This enum lists all the operations that can be executed on a given timer."
  }
}

-- Functions
funcs = 
{
  { sig = "int platform_timer_exists( unsigned id );",
    desc = [[Checks if the platform has the timer specified as argument. Implemented in %src/common.c%, it uses the $NUM_TIMER$ macro that must be defined in the
platform's $platform_conf.h$ file (see @arch_overview.html#platforms@here@ for details) and the virtual timer configuration (@#virtual@here@ for details). For example:</p>
~#define NUM_TIMER   2      $// The platform has 2 hardware timers$~<p>]],
    args = 
    {
      { name = "id", desc = "the timer ID" },
    },
    ret = "1 if the timer exists, 0 otherwise"
  },

  { sig = "void platform_timer_delay( unsigned id, u32 delay_us );",
    link = "platform_timer_delay",
    desc = [[Waits on a timer, then returns. This function is "split" in two parts: a platform-independent part implemented in %src/common.c% (that
handles virtual timers) and a platform-dependent part that must be implemented by each platform in a function named @#platform_s_timer_delay@platform_s_timer_delay@. This function handles both
hardware timer IDs and virtual timer IDs.<br>
<font color="red"><a name="limitations">$IMPORTANT NOTE$</font></a>: the real delay after executing this functions depends a number of variables, most notably the base clock of the timer 
and the size of the timer counter register (32 bits on some platforms, 16 bits on most platforms, other values are less common). To ensure that the delay you're requesting is achievable, use 
@#platform_timer_op@platform_timer_op@ with $PLATFORM_TIMER_OP_GET_MAX_DELAY$ and $PLATFORM_TIMER_OP_GET_MIN_DELAY$ to obtain the maximum and the minimum 
achievable wait times on your timer, respectively. Even if your delay is within these limits, the $precision$ of this function still varies a lot, mainly as a function of 
the timer base clock.]],
    args = 
    {
      { name = "id", desc = "the timer ID" },
      { name = "delay_us", desc = "the delay time (in microseconds)" }
    }
  },

  { sig = "void platform_s_timer_delay( unsigned id, u32 delay_us );",
    link = "platform_s_timer_delay",
    desc = [[This function is identical in functionality to @#platform_timer_delay@platform_timer_delay@, but this is the function that must actually be implemented by a platform port, 
and it must never handle virtual timer IDs, only hardware timer IDs. It has the same @#limitations@limitations@ as @#platform_timer_delay@platform_timer_delay@.]],
    args = 
    {
      { name = "id", desc = "the timer ID" },
      { name = "delay_us", desc = "the delay time (in microseconds)" }
    }
  },

  { sig = "u32 platform_timer_op( unsigned id, int op, u32 data );",
    link = "platform_timer_op",
    desc = [[Executes an operation on a timer. This function is "split" in two parts: a platform-independent part implemented in %src/common.c% (that handles virtual timers) and a 
platform-dependent part that must be implemented by each platform in a function named @#platform_s_timer_op@platform_s_timer_op@. This function handles both hardware timer IDs and virtual 
timer IDs.]],
    args = 
    {
      { name = "id", desc = "the timer ID" },
      { name = "op", desc = [[the operation. $op$ can take any value from the @#opval@this enum@, as follows:
<ul>
  <li>$PLATFORM_TIMER_OP_START$: start the specified timer by setting its counter register to a predefined value.</li>
  <li>$PLATFORM_TIMER_OP_READ$: get the value of the specified timer's counter register.</li> 
  <li>$PLATFORM_TIMER_SET_CLOCK$: set the clock of the specified timer to $data$ (in hertz). You can never set the clock of a virtual timer, which is set at compile time.</li>  
  <li>$PLATFORM_TIMER_GET_CLOCK$: get the clock of the specified timer.</li>  
  <li>$PLATFORM_TIMER_OP_GET_MAX_DELAY$: get the maximum achievable timeout on the specified timer (in us).</li>  
  <li>$PLATFORM_TIMER_OP_GET_MIN_DELAY$: get the minimum achievable timeout on the specified timer (in us).</li>
</ul>]] },
      { name = "data", desc = "used to specify the timer clock value when $op = PLATFORM_TIMER_SET_CLOCK$, ignored otherwise" },
    },
    ret = 
    {
      "the predefined value used when starting the clock if $op = PLATFORM_TIMER_OP_START$",
      "the timer's counter register if $op = PLATFORM_TIMER_OP_READ$",
      "the actual clock set on the timer, which might be different than the request clock depending on the hardware if $op = PLATFORM_TIMER_SET_CLOCK$",
      "the timer clock if $op = PLATFORM_TIMER_GET_CLOCK$", 
      "the maximum achievable delay (in microseconds) if $op = PLATFORM_TIMER_OP_GET_MAX_DELAY$",
      "the minimum achievable delay (in microseconds) if $op = PLATFORM_TIMER_OP_GET_MIN_DELAY$"
    }
  }, 

  { sig = "u32 platform_s_timer_op( unsigned id, int op, u32 data );",
    link = "platform_s_timer_op",
    desc = [[This function is identical in functionality to @#platform_timer_op@platform_timer_op@, but this is the function that must actually be implemented by a platform port, and it must 
never handle virtual timer IDs, only hardware timer IDs.]],
    args = 
    {
      { name = "id", desc = "the timer ID" },
      { name = "op", desc = [[the operation. $op$ can take any value from the @#opval@this enum@, as follows:
<ul>
  <li>$PLATFORM_TIMER_OP_START$: start the specified timer by setting its counter register to a predefined value.</li>
  <li>$PLATFORM_TIMER_OP_READ$: get the value of the specified timer's counter register.</li> 
  <li>$PLATFORM_TIMER_SET_CLOCK$: set the clock of the specified timer to $data$ (in hertz). You can never set the clock of a virtual timer, which is set at compile time.</li>  
  <li>$PLATFORM_TIMER_GET_CLOCK$: get the clock of the specified timer.</li>  
  <li>$PLATFORM_TIMER_OP_GET_MAX_DELAY$: get the maximum achievable timeout on the specified timer (in us).</li>  
  <li>$PLATFORM_TIMER_OP_GET_MIN_DELAY$: get the minimum achievable timeout on the specified timer (in us).</li>
</ul>]] },
      { name = "data", desc = "used to specify the timer clock value when $op = PLATFORM_TIMER_SET_CLOCK$, ignored otherwise" },
    },
    ret = 
    {
      "the predefined value used when starting the clock if $op = PLATFORM_TIMER_OP_START$",
      "the timer's counter register if $op = PLATFORM_TIMER_OP_READ$",
      "the actual clock set on the timer, which might be different than the request clock depending on the hardware if $op = PLATFORM_TIMER_SET_CLOCK$",
      "the timer clock if $op = PLATFORM_TIMER_GET_CLOCK$", 
      "the maximum achievable delay (in microseconds) if $op = PLATFORM_TIMER_OP_GET_MAX_DELAY$",
      "the minimum achievable delay (in microseconds) if $op = PLATFORM_TIMER_OP_GET_MIN_DELAY$"
    }
  }, 

  { sig = "u32 platform_timer_get_diff_us( unsigned id, timer_data_type end, timer_data_type start );",
    desc = [[Return the time difference (in us) betweeen two timer values. This function is generic for all platforms, thus it is implemented in %src/common.c%.]],
    args = 
    {
      { name = "id", desc = "the timer ID" },
      { name = "end", desc = "the first timer value" },
      { name = "start", desc = "the second timer value" },
    },
    ret = "the time difference (in microseconds)"
  }
}

auxdata = 
{
  { title = "Virtual timers",
    link = "virtual",
    desc = 
[[$Virtual timers$ were added to eLua to overcome some limitations:
<ul>
  <li>there are generally few hardware timers available, some of which might be dedicated (thus not usable directly by eLua).</li>
  <li>many times it is difficult to share a hardware timer between different parts of an application because of conflicting requirements. Generally it's not possible to have timers that can
      achieve long delays and high accuracy at the same time (this is especially true for systems that have 16 bit or even smaller timers).</li>
</ul></p>
<p>In this respect, $virtual timers$ are a set of timers that share a single hardware timer. It is possible, in this way, to have a hardware timer that can implement 4, 8 or more hardware
timers. There are a few drawbacks to this approach:
<ul>
  <li>the hardware timer used to implement the virtual timers must generally be dedicated. In fact in cat be still used in "read only mode", which means that the only operations that can
      be executed on it are $PLATFORM_TIMER_OP_READ$, $PLATFORM_TIMER_GET_CLOCK$, $PLATFORM_TIMER_OP_GET_MAX_DELAY$ and $PLATFORM_TIMER_OP_GET_MIN_DELAY$. However,
      since the "read only mode" is not enforced by the code, it is advisable to treat this timer as a dedicated resource and thus make it invisible to eLua by not associating it with 
      an ID.</li>
  <li>the number of virtual timers and their base frequency are fixed at compile time.</li>
  <li>virtual timers are generally used for large delays with low accuracy, since their base frequency should be fairly low (see below).</li>
</ul></p>
<p>To $enable$ virtual timers:
<ol>
  <li>edit $platform_conf.h$ (see @arch_overview.html#platforms@here@ for details) and set $VTMR_NUM_TIMERS$ to the number of desired virtual timers and 
     $VTMR_FREQ_HZ$ to the base frequency of the virtual timers (in hertz). For example:
~#define VTMR_NUM_TIMERS       4 // we need 4 virtual timers
#define VTMR_FREQ_HZ          4 // the base clock for the virtual timers is 4Hz~</li>
  <li>in your platform port setup a hardware timer to fire an interrupt at $VTMR_FREQ_HZ$ and call the $cmn_virtual_timer_cb$ function (defined in %src/common.c%) in the 
     timer interrupt handler. For example, if the the interrupt handler is called $timer_int_handler$, do this:
~void timer_int_handler( void )
{
  // add code to clear the timer interrupt flag here if needed
  cmn_virtual_timer_cb();
}~</li>
</ol></p>
<p>Note that because of step 2 above you are limited by practical constraints on the value of $VTMR_FREQ_HZ$. If set too high, the timer interrupt will fire too often, thus taking too much
CPU time. The maximum value depends largely on the hardware and the desired behaviour of the virtual timers, but in practice values larger than 10 might visibly change the behaviour of your 
system.</p>
<p>To $use$ a virtual timer, identify it with the constant $VTMR_FIRST_ID$ (defined in %inc/common.h%) plus an offset. For example, $VTMR_FIRST_ID+0$ (or simply
$VTMR_FIRST_ID$) is the ID of the first virtual timer in the system, and $VTMR_FIRST_ID+2$ is the ID of the third virtual timer in the system.</p>
]]
  }
}

