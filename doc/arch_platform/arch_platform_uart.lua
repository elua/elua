-- eLua platform interface - UART

-- Title
title = "eLua platform interface - UART"

-- Overview
overview = "This part of the platform interface groups functions related to the UART interface(s) of the MCU."

-- Data structures, constants and types
structures = 
{
  { text = [[// Parity
enum
{
  PLATFORM_UART_PARITY_EVEN,
  PLATFORM_UART_PARITY_ODD,
  PLATFORM_UART_PARITY_NONE
};]],
    link = "parval",
    desc = "Constants used to specify the UART parity mode."
  },

  { text = [[// Stop bits
enum
{
  PLATFORM_UART_STOPBITS_1,
  PLATFORM_UART_STOPBITS_1_5,
  PLATFORM_UART_STOPBITS_2
};]],
    link = "stopval",
    desc = "Constants used to specify the number of UART stop bits.",
  },

  { text = [[// "Infinite timeout" constant for recv
#define PLATFORM_UART_INFINITE_TIMEOUT        (-1)]],
    link = "toinf",
    desc = "This constant is used as a special timeout value (infinite timeout) in the UART functions that expect a timeout as argument.",
  }
}

-- Functions
funcs = 
{
  { sig = "int platform_uart_exists( unsigned id );",
    desc = [[Checks if the platform has the hardware UART specified as argument. Implemented in %src/common.c%, it uses the $NUM_UART$ macro that must be defined in the
platform's $platform_conf.h$ file (see @arch_overview.html#platforms@here@ for details). For example:</p>
~#define NUM_UART   2      $// The platform has 2 UART interfaces$~<p>]],
    args = 
    {
      { name = "id", desc = "UART interface ID" },
    },
    ret = "1 if the specified UART exists, 0 otherwise"
  },

  { sig = "u32 platform_uart_setup( unsigned id, u32 baud, int databits, int parity, int stopbits );",
    desc = "This function is used to initialize the parameters of the UART interface.",
    args = 
    {
      { name = "id", desc = "UART interface ID." },
      { name = "baud", desc = "baud rate." },
      { name = "databits", desc = "number of databits (maximum 8)." },
      { name = "parity", desc = "parity type (can be either $PLATFORM_UART_PARITY_EVEN$, $PLATFORM_UART_PARITY_ODD$ or $PLATFORM_UART_PARITY_NONE$, see @#parval@here@)." },
      { name = "stopbits", desc = [[number of stop bits (can be either $PLATFORM_UART_STOPBITS_1$, $PLATFORM_UART_STOPBITS_1_5$ or $PLATFORM_UART_STOPBITS_2$, see
         @#stopval@here@).]] },
    },
    ret = "the actual baud rate. Depending on the hardware, this may have a different value than the $baud$ argument.",
  },

  { sig = "void platform_uart_send( unsigned id, u8 data );",
    desc = "Send data to an UART interface.",
    args = 
    {
      { name = "id", desc = "UART interface ID." },
      { name = "data", desc = "data to be sent." },
    },
  },

  { sig = "int platform_uart_recv( unsigned id, unsigned timer_id, s32 timeout );",
    link = "platform_uart_recv",
    desc = [[Receive data from the UART interface (blocking/non blocking with timeout/immediate).<br>
This function is "split" in two parts: a platform-independent part that is implemented in %src/common.c%, and a platform-dependent part that must be implemented by each
platform in a function named @#platform_s_uart_recv@platform_s_uart_recv@.]],
    args = 
    {
      { name = "id", desc = "UART interface ID." },
      { name = "timer_id", desc = "the ID of the timer used in this operation (see @arch_platform_timers.html@here@ for details). See also the description of the $timeout$ argument." },
      { name = "timeout", desc = [[specifies a timeout for the receive operation as follows:
<ul>
  <li>$timeout &gt; 0$: the timer with the specified $timer_id$ will be used to timeout the receive operation after $timeout$ microseconds.</li>
  <li>$timeout = 0$: the function returns immediately regardless of data being available or not. $timer_id$ is ignored.</li>
  <li>$timeout$ = @#toinf@PLATFORM_UART_INFINITE_TIMEOUT@: the function waits indefinitely for UART data to be available and returns it. In this mode the function doesn't 
      time out, so $timer_id$ is ignored.</li>
</ul>]] },
    },
    ret = 
    {
      "if $timeout &gt; 0$ and data from the UART is available in $timeout$ microseconds of less it is returned, otherwise -1 is returned",
      "if $timeout = 0$ and data from the UART is available when the function is called it is returned, otherwise -1 is returned",
      "if $timeout$ = @#toinf@PLATFORM_UART_INIFINITE_TIMEOUT@ it returns the data read from the UART after it becomes available"
    }
  },

  { sig = "int platform_s_uart_recv( unsigned id, s32 timeout );",
    link = "platform_s_uart_recv",
    desc = [[This is the platform-dependent part of the UART receive function @#platform_uart_recv@platform_uart_recv@, and is in fact a "subset" of the full function 
(thus being easier to implement by each platform in part). In particular, it never needs to deal with the $timeout &gt; 0$ case, which is handled by @#platform_uart_recv@platform_uart_recv@.]],
     args = 
    {
      { name = "id", desc = "UART interface ID." },
      { name = "timeout", desc = [[specifies a timeout for the receive operation as follows:
<ul>
  <li>$timeout = 0$: the function returns immediately regardless of data being available or not.</li>
  <li>$timeout$ = @#toinf@PLATFORM_UART_INFINITE_TIMEOUT@: the function waits indefinitely for UART data to be available and returns it.</li>
</ul>]] },
    },
    ret = 
    {
      "if $timeout = 0$ and data from the UART is available when the function is called it is returned, otherwise -1 is returned",
      "if $timeout$ = @#toinf@PLATFORM_UART_INIFINITE_TIMEOUT@ it returns the data read from the UART after it becomes available"
    }
  }
}

