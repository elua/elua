-- eLua platform interface - UART

data_en = 
{
  -- Title
  title = "eLua platform interface - UART",

  -- Menu name
  menu_name = "UART",

  -- Overview
  overview = "This part of the platform interface groups functions related to the UART interface(s) of the MCU.",

  -- Data structures, constants and types
  structures = 
  {
    { text = [[// Parity
enum
{
  PLATFORM_UART_PARITY_EVEN,
  PLATFORM_UART_PARITY_ODD,
  PLATFORM_UART_PARITY_NONE,
  PLATFORM_UART_PARITY_MARK,
  PLATFORM_UART_PARITY_SPACE
};]],
      name = "UART parity",
      desc = "Constants used to specify the UART parity mode."
    },

    { text = [[// Stop bits
enum
{
  PLATFORM_UART_STOPBITS_1,
  PLATFORM_UART_STOPBITS_1_5,
  PLATFORM_UART_STOPBITS_2
};]],
      name = "UART stop bits",
      desc = "Constants used to specify the number of UART stop bits.",
    },

     { text = [[// Virtual UART IDs
#define SERMUX_SERVICE_ID_FIRST  0xD0
#define SERMUX_SERVICE_ID_LAST   0xD7
]],
      name = "Virtual UART IDs",
      desc = "If @sermux.html@virtual UART@ support is enabled these constants define the IDs of the virtual UARTs in the system (defined in %inc/sermux.h%).",
    },

     { text = [[// Flow control type
#define PLATFORM_UART_FLOW_NONE               0
#define PLATFORM_UART_FLOW_RTS                1
#define PLATFORM_UART_FLOW_CTS                2
]],
      name = "Flow control type",
      desc = "Used to set the flow control type on a serial interface. These constans can be ORed together ($PLATFORM_UART_FLOW_RTS | PLATFORM_UART_FLOW_CTS$)",
    },

   
  },

  -- Functions
  funcs = 
  {
    { sig = "int #platform_uart_exists#( unsigned id );",
      desc = [[Checks if the platform has the hardware UART specified as argument. Implemented in %src/common.c%, it uses the $NUM_UART$ macro that must be defined in the
  platform's $cpu_xxx.h$ file (see @arch_overview.html#platforms@here@ for details). For example:</p>
  ~#define NUM_UART   2      $// The platform has 2 UART interfaces$~<p>]],
      args = "$id$ - UART interface ID",
      ret = "1 if the specified UART exists, 0 otherwise"
    },

    { sig = "u32 #platform_uart_setup#( unsigned id, u32 baud, int databits, int parity, int stopbits );",
      desc = "This function is used to initialize the parameters of the UART interface.",
      args = 
      {
        "$id$ - UART interface ID.",
        "$baud$ - baud rate.",
        "$databits$ - number of databits (maximum 8).",
        "$parity$ - parity type (can be either $PLATFORM_UART_PARITY_EVEN$, $PLATFORM_UART_PARITY_ODD$, $PLATFORM_UART_PARITY_MARK$, $PLATFORM_UART_PARITY_SPACE$ or $PLATFORM_UART_PARITY_NONE$, see @#uart_parity@here@).",
        [[$stopbits$ - number of stop bits (can be either $PLATFORM_UART_STOPBITS_1$, $PLATFORM_UART_STOPBITS_1_5$ or $PLATFORM_UART_STOPBITS_2$, see
           @#uart_stop_bits@here@).]],
      },
      ret = "the actual baud rate. Depending on the hardware, this may have a different value than the $baud$ argument.",
    },

    { sig = "void #platform_uart_send#( unsigned id, u8 data );",
      desc = [[Send data to an UART interface. This is a blocking operation (it doesn't return until the data was sent).<br>
      This function is "split" in two parts: a platform-independent part that is implemented in %src/common.c% and a platform-dependent part that must be implemented
      by each platform in a function named @#platform_s_uart_send@platform_s_uart_send@.]],
      args = 
      {
        "$id$ - UART interface ID.",
        "$data$ - data to be sent.",
      },
    },

    { sig = "void #platform_s_uart_send#( unsigned id, u8 data );",
      desc = [[This is the platform-dependent part of @#platform_uart_send@platform_uart_send@. It doesn't need to take care of @sermux.html@virtual UARTs@ or other system
      configuration parameters, it just needs to instruct the CPU to send the data on the specified ID. This function will always be called with a physical uart ID.]],
      args = 
      {
        "$id$ - UART interface ID.",
        "$data$ - data to be sent.",
      },
    },

    { sig = "int #platform_uart_recv#( unsigned id, unsigned timer_id, timer_data_type timeout );",
      link = "platform_uart_recv",
      desc = [[Receive data from the UART interface (blocking/non blocking with timeout/immediate).<br>
  This function is "split" in two parts: a platform-independent part that is implemented in %src/common.c% and a platform-dependent part that must be implemented by each
  platform in a function named @#platform_s_uart_recv@platform_s_uart_recv@.]],
      args = 
      {
        "$id$ - UART interface ID.",
        "$timer_id$ - the ID of the timer used in this operation (see @arch_platform_timers.html@here@ for details). See also the description of the $timeout$ argument.",
        [[$timeout$ - specifies a timeout for the receive operation as follows:
  <ul>
    <li>$timeout > 0$: the timer with the specified $timer_id$ will be used to timeout the receive operation after $timeout$ microseconds.</li>
    <li>$timeout = 0$: the function returns immediately regardless of data being available or not. $timer_id$ is ignored.</li>
    <li>$timeout = PLATFORM_TIMER_INF_TIMEOUT$: the function waits indefinitely for UART data to be available and returns it. In this mode the function doesn't 
        time out, so $timer_id$ is ignored.</li>
  </ul>]],
      },
      ret = 
      {
        "if $timeout > 0$ and data from the UART is available in $timeout$ microseconds of less it is returned, otherwise -1 is returned",
        "if $timeout = 0$ and data from the UART is available when the function is called it is returned, otherwise -1 is returned",
        "if $timeout = PLATFORM_TIMER_INF_TIMEOUT$ it returns the data read from the UART after it becomes available"
      }
    },

    { sig = "int #platform_s_uart_recv#( unsigned id, timer_data_type timeout );",
      link = "platform_s_uart_recv",
      desc = [[This is the platform-dependent part of the UART receive function @#platform_uart_recv@platform_uart_recv@ and is in fact a "subset" of the full function 
  (thus being easier to implement by each platform in part). In particular, it never needs to deal with the $timeout &gt; 0$ case, which is handled by @#platform_uart_recv@platform_uart_recv@.]],
       args = 
      {
        "$id$ - UART interface ID.",
        [[$timeout$ - specifies a timeout for the receive operation as follows:
  <ul>
    <li>$timeout = 0$: the function returns immediately regardless of data being available or not.</li>
    <li>$timeout = PLATFORM_TIMER_INF_TIMEOUT$: the function waits indefinitely for UART data to be available and returns it.</li>
  </ul>]],
      },
      ret = 
      {
        "if $timeout = 0$ and data from the UART is available when the function is called it is returned, otherwise -1 is returned",
        "if $timeout = PLATFORM_TIMER_INF_TIMEOUT$ it returns the data read from the UART after it becomes available"
      }
    },

    { sig = "int #platform_uart_set_buffer#( unsigned id, unsigned log2size );",
      desc = "Sets the buffer for the specified UART. This function is fully implemented in %src/common.c%.",
      args = 
      {
        "$id$ - UART interface ID.",
        "$data$ - the base 2 logarithm of the buffer size or 0 to disable buffering on the UART. Note that disabling buffering on a virtual UART is an invalid operation."
      },
      ret = "$PLATFORM_OK$ if the operation succeeded, $PLATFORM_ERR$ otherwise."
    },

     { sig = "int #platform_uart_set_flow_control#( unsigned id, int type );",
      desc = [[Sets the flow control type.<br>
      This function is "split" in two parts: a platform independent part that is implemented in %src/common.c% and a platform-dependent part that must be implemented by each
      platform in a function named @#platform_s_uart_set_flow_control@platform_s_uart_set_flow_control@.]],
      args = 
      {
        "$id$ - UART interface ID.",
        [[$type$ - the desired flow control. It can be either $PLATFORM_UART_FLOW_NONE$, $PLATFORM_UART_FLOW_RTS$ or $PLATFORM_UART_FLOW_CTS$ or a bitwise combination of these constants
        (see @#flow_control_type@here@ for details).]]
      },
      ret = "$PLATFORM_OK$ if the operation succeeded, $PLATFORM_ERR$ otherwise."
    },
   
      { sig = "int #platform_s_uart_set_flow_control#( unsigned id, int type );",
      desc = [[This is the platform-dependent part of the UART set flow control function @#platform_uart_set_flow_control@platform_uart_set_flow_control@ and is in fact a "subset" of the
      full function (thus being easier to implement by each platform in part). In particular, it never needs to deal with virtual UARTs.]],
      args = 
      {
        "$id$ - UART interface ID.",
        [[$type$ - the desired flow control. It can be either $PLATFORM_UART_FLOW_NONE$, $PLATFORM_UART_FLOW_RTS$ or $PLATFORM_UART_FLOW_CTS$ or a bitwise combination of these constants
        (see @#flow_control_type@here@ for details).]]
      },
      ret = "$PLATFORM_OK$ if the operation succeeded, $PLATFORM_ERR$ otherwise."
    },
  
  }
}

