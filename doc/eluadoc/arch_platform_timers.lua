-- eLua platform interface - timers

data_en = 
{
  -- Title
  title = "eLua platform interface - timers",

  -- Menu name
  menu_name = "Timers",

  -- Overview
  overview = [[This part of the platform interface groups functions related to the timers of the MCU. It also makes provisions for using $virtual timers$ on any platform, see @#virtual@this section@
  for details. Keep in mind that in the following paragraphs a $timer id$ can refer to both a hardware timer or a virtual timer.]],

  -- Data structures, constants and types
  structures = 
  {
    { text = "typedef u64/u32 timer_data_type;",
      name = "Timer data type",
      desc = [[This defines the data type used to specify delays and time intervals (which are always specified in $microseconds$). The choice between u64 and u32 for the timer data type depends
on the build type, check ^#the_system_timer^here^ for mode details.]]
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
      name = "Timer operations",
      desc = "This enum lists all the operations that can be executed on a given timer."
    }
  },

  -- Functions
  funcs = 
  {
    { sig = "int #platform_timer_exists#( unsigned id );",
      desc = [[Checks if the platform has the timer specified as argument. Implemented in %src/common.c%, it uses the $NUM_TIMER$ macro that must be defined in the
  platform's $platform_conf.h$ file (see @arch_overview.html#platforms@here@ for details) and the virtual timer configuration (@#virtual@here@ for details). For example:</p>
  ~#define NUM_TIMER   2      $// The platform has 2 hardware timers$~<p>]],
      args = "$id$ - the timer ID",
      ret = "1 if the timer exists, 0 otherwise"
    },

    { sig = "void #platform_timer_delay#( unsigned id, u32 delay_us );",
      desc = [[Waits on a timer, then returns. This function is "split" in two parts: a platform-independent part implemented in %src/common.c% (that
  handles virtual timers) and a platform-dependent part that must be implemented by each platform in a function named @#platform_s_timer_delay@platform_s_timer_delay@. This function handles both
  hardware timer IDs and virtual timer IDs.<br>
  <a name="limitations" /><span class="warning">IMPORTANT NOTE</span>: the real delay after executing this functions depends on a number of variables, most notably the base clock of the timer 
  and the size of the timer counter register (32 bits on some platforms, 16 bits on most platforms, other values are less common). To ensure that the delay you're requesting is achievable, use 
  @#platform_timer_op@platform_timer_op@ with $PLATFORM_TIMER_OP_GET_MAX_DELAY$ and $PLATFORM_TIMER_OP_GET_MIN_DELAY$ to obtain the maximum and the minimum 
  achievable wait times on your timer, respectively. Even if your delay is within these limits, the $precision$ of this function still varies a lot, mainly as a function of 
  the timer base clock.]],
      args = 
      {
        "$id$ - the timer ID",
        "$delay_us$ - the delay time (in microseconds)"
      }
    },

    { sig = "void #platform_s_timer_delay#( unsigned id, u32 delay_us );",
      desc = [[This function is identical in functionality to @#platform_timer_delay@platform_timer_delay@, but this is the function that must actually be implemented by a platform port, 
  and it must never handle virtual timer IDs, only hardware timer IDs. It has the same @#limitations@limitations@ as @#platform_timer_delay@platform_timer_delay@.]],
      args = 
      {
        "$id$ - the timer ID",
        "$delay_us$ - the delay time (in microseconds)"
      }
    },

    { sig = "u32 #platform_timer_op#( unsigned id, int op, u32 data );",
      desc = [[Executes an operation on a timer. This function is "split" in two parts: a platform-independent part implemented in %src/common.c% (that handles virtual timers) and a 
  platform-dependent part that must be implemented by each platform in a function named @#platform_s_timer_op@platform_s_timer_op@. This function handles both hardware timer IDs and virtual 
  timer IDs.]],
      args = 
      {
        "$id$ - the timer ID",
        [[$op$ - the operation. $op$ can take any value from the @#timer_operations@this enum@, as follows:
  <ul>
    <li>$PLATFORM_TIMER_OP_START$: start the specified timer by setting its counter register to a predefined value.</li>
    <li>$PLATFORM_TIMER_OP_READ$: get the value of the specified timer's counter register.</li> 
    <li>$PLATFORM_TIMER_SET_CLOCK$: set the clock of the specified timer to $data$ (in hertz). You can never set the clock of a virtual timer, which is set at compile time.</li>  
    <li>$PLATFORM_TIMER_GET_CLOCK$: get the clock of the specified timer.</li>  
    <li>$PLATFORM_TIMER_OP_GET_MAX_DELAY$: get the maximum achievable timeout on the specified timer (in us).</li>  
    <li>$PLATFORM_TIMER_OP_GET_MIN_DELAY$: get the minimum achievable timeout on the specified timer (in us).</li>
  </ul>]],
        "$data$ - used to specify the timer clock value when $op = PLATFORM_TIMER_SET_CLOCK$, ignored otherwise",
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

    { sig = "u32 #platform_s_timer_op#( unsigned id, int op, u32 data );",
      desc = [[This function is identical in functionality to @#platform_timer_op@platform_timer_op@, but this is the function that must actually be implemented by a platform port, and it must 
  never handle virtual timer IDs, only hardware timer IDs.]],
      args = 
      {
        "$id$ - the timer ID",
        [[$op$ - the operation. $op$ can take any value from the @#opval@this enum@, as follows:
  <ul>
    <li>$PLATFORM_TIMER_OP_START$: start the specified timer by setting its counter register to a predefined value.</li>
    <li>$PLATFORM_TIMER_OP_READ$: get the value of the specified timer's counter register.</li> 
    <li>$PLATFORM_TIMER_SET_CLOCK$: set the clock of the specified timer to $data$ (in hertz). You can never set the clock of a virtual timer, which is set at compile time.</li>  
    <li>$PLATFORM_TIMER_GET_CLOCK$: get the clock of the specified timer.</li>  
    <li>$PLATFORM_TIMER_OP_GET_MAX_DELAY$: get the maximum achievable timeout on the specified timer (in us).</li>  
    <li>$PLATFORM_TIMER_OP_GET_MIN_DELAY$: get the minimum achievable timeout on the specified timer (in us).</li>
  </ul>]],
        "$data$ - used to specify the timer clock value when $op = PLATFORM_TIMER_SET_CLOCK$, ignored otherwise",
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

    { sig = "u32 #platform_timer_get_diff_us#( unsigned id, timer_data_type end, timer_data_type start );",
      desc = [[Return the time difference (in us) between two timer values. This function is generic for all platforms, thus it is implemented in %src/common.c%.]],
      args = 
      {
        "$id$ - the timer ID",
        "$end$ - the first timer value",
        "$start$ - the second timer value",
      },
      ret = "the time difference (in microseconds)"
    },

    { sig = "int #platform_timer_set_match_int#( unsigned id, u32 period_us, int type );",
      desc = "Setup the timer match interrupt. Only available if interrupt support is enabled, check @inthandlers.html@here@ for details.",
      args = 
      {
        "$id$ - the timer ID",
        "$period_us$ - the period (in microseconds) of the timer interrupt. Setting this to 0 disables the timer match interrupt.",
        [[$type$ - $PLATFORM_TIMER_INT_ONESHOT$ for an interrupt that occurs only once after $period_us$ microseconds, or $PLATFORM_TIMER_INT_CYCLIC$ for an interrupt that occurs every
$period_us$ microseconds]]        
      },
      ret = 
      {
        "$PLATFORM_TIMER_INT_OK$ if the operation was successful.",
        "$PLATFORM_TIMER_INT_TOO_SHORT$ if the specified period is too short.",
        "$PLATFORM_TIMER_INT_INVALID_ID$ if the specified timer cannot handle this operation."
      }
    }
  },

  auxdata = 
  {
    { title = "Virtual timers",
      desc = 
  [[$Virtual timers$ were added to eLua to overcome some limitations:</p>
  <ul>
    <li>there are generally few hardware timers available, some of which might be dedicated (thus not usable directly by eLua).</li>
    <li>many times it is difficult to share a hardware timer between different parts of an application because of conflicting requirements. Generally it's not possible to have timers that can
        achieve long delays and high accuracy at the same time (this is especially true for systems that have 16 bit or even smaller timers).</li>
  </ul>
  <p>In this respect, $virtual timers$ are a set of timers that share a single hardware timer. It is possible, in this way, to have a hardware timer that can implement 4, 8 or more virtual/software
  timers. There are a few drawbacks to this approach:</p>
  <ul>
    <li>the hardware timer used to implement the virtual timers must generally be dedicated. In fact it can still be used in "read only mode", which means that the only operations that can
        be executed on it are $PLATFORM_TIMER_OP_READ$, $PLATFORM_TIMER_GET_CLOCK$, $PLATFORM_TIMER_OP_GET_MAX_DELAY$ and $PLATFORM_TIMER_OP_GET_MIN_DELAY$. However,
        since the "read only mode" is not enforced by the code, it is advisable to treat this timer as a dedicated resource and thus make it invisible to eLua by not associating it with 
        an ID.</li>
    <li>the number of virtual timers and their base frequency are fixed at compile time.</li>
    <li>virtual timers are generally used for large delays with low accuracy, since their base frequency should be fairly low (see below).</li>
  </ul>
  <p>To $enable$ virtual timers:</p>
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
  </ol>
  <p>Note that because of step 2 above you are limited by practical constraints on the value of $VTMR_FREQ_HZ$. If set too high, the timer interrupt will fire too often, thus taking too much
  CPU time. The maximum value depends largely on the hardware and the desired behaviour of the virtual timers, but in practice values larger than 10 might visibly change the behaviour of your 
  system.</p>
  <p>To $use$ a virtual timer, identify it with the constant $VTMR_FIRST_ID$ (defined in %inc/common.h%) plus an offset. For example, $VTMR_FIRST_ID+0$ (or simply
  $VTMR_FIRST_ID$) is the ID of the first virtual timer in the system, and $VTMR_FIRST_ID+2$ is the ID of the third virtual timer in the system.</p>
  <p>Virtual timers are capable of generating timer match interrupts just like regular timers, check @#platform_timer_set_match_int@here@ for details.
  ]]
    },
    { title = "The system timer",
    desc = 
  [[The system timer was introduced in eLua 0.9 as a simpler alternative to the traditional eLua timers. Working with regular timers in eLua might be challenging for a number of reasons:</p>
  <ul>
    <li>depending on the hardware, the timers might have a limited range. Because of this, they might not be able to timeout in the interval requested by the user.</li>
    <li>the timers might have different ranges even on the same platform (they might have a different base clock, for example). The problem is further aggravated when switching platforms.</li>
    <li>the timers might be shared with other hardware resources (for example PWMs or ADC triggers) so using them might have unexpected side effects.</li>
    <li>manual timer management is error prone. The user needs to keep into account the timers he's using, their base frequencies and wether they are shared or not with the C code.</li>
  </ul>
  <p>The ^#virtual_timers^virtual timers^ can fix some of the above problems, but their resolution is fairly low and they still require manual management.</p>
  <p>The $system timer$ attemps to fix (at least partially) these issues. It is a timer with fixed resolution (1us) %on all platforms% and large counters:</p>
  <ul>
    <li>if eLua is compiled in floating point mode (default) the counter is 52 bits wide. It will overflow after more than 142 %years%.</li>
    <li>if eLua is compiled in 32 bit integer-only mode (lualong) the counter is 32 bits wide. It will overflow after about one hour.</li>
    <li>if eLua is compiled in 64 bit integer-only mode (lualonglong, new in 0.9) the counter is again 52 bits wide and it will also overflow after more than 142 years.</li>
  </ul>
  <p>The eLua API was partially modified to take full advantage of this new timer:</p>
  <ul>
    <li>all the functions that can operate with a timeout (for example @refman_gen_uart.html#uart.read@uart.read@ or @refman_gen_net.html#net.accept@net.accept@) 
will default to the system timer is a timer ID is not specified explicitly.</li>
    <li>all the function in the @refman_gen_tmr.html@timer module@ will default to the system timer if a timer ID is not specified explicitly.</li>
    <li>timeouts are specified in a more unified manner across the eLua modules as a $[timeout], [timer_id]$ pair:
  <table class="table_center" style="margin-top: 10px; margin-bottom: 4px;">
  <tbody>
  <tr>
    <th>timeout</th>
    <th>timer_id</th>
    <th>Result</th>
  </tr>
  <tr>
    <td>not specified</td>
    <td>any value</td>
    <td>infinite timeout (the function blocks until it completes).</td>
  </tr>
  <tr>
    <td>0</td>
    <td>any value</td>
    <td>no timeout (the function returns immediately).<//td>
  </tr>
  <tr>
    <td>a positive value</td>
    <td>not specified</td>
    <td>the system timer will be used to measure the function's timeout.</td>
  </tr>  
  <tr>
    <td>a positive value</td>
    <td>a timer ID</td>
    <td>the specified timer will be used to measure the function's timeout.</td>
  </tr>
  </tbody>
  </table>
  </li>  
  </ul>
  <p>Using the system timer as much as possible is also encouraged with C code that uses the eLua C api, not only with Lua programs. The C code can use the system timer by specifying 
$PLATFORM_TIMER_SYS_ID$ as the timer ID.]]
  }
 }
}


