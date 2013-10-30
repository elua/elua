-- eLua platform interface - PIO

data_en = 
{
  -- Title
  title = "eLua platform interface - PIO",

  -- Menu name
  menu_name = "PIO",

  -- OverviewA
  overview = "This part of the platform interface deals with PIO (Programmable Input Output) operations, thus letting the user access the low level input/output facilities of the host MCU.",

  -- Data structures, constants and types
  structures = 
  {
    { text = [[enum
{
  // Pin operations
  PLATFORM_IO_PIN_SET,                  $// Set the pin to 1$
  PLATFORM_IO_PIN_CLEAR,                $// Clear the pin (set it to 0)$
  PLATFORM_IO_PIN_GET,                  $// Get the value of the pin$
  PLATFORM_IO_PIN_DIR_INPUT,            $// Make the pin an input$
  PLATFORM_IO_PIN_DIR_OUTPUT,           $// Make the pin an output$
  PLATFORM_IO_PIN_PULLUP,               $// Activate the pullup on the pin$
  PLATFORM_IO_PIN_PULLDOWN,             $// Activate the pulldown on the pin$
  PLATFORM_IO_PIN_NOPULL,               $// Disable all pullups/pulldowns on the pin$
  // Port operations
  PLATFORM_IO_PORT_SET_VALUE,           $// Set port value$
  PLATFORM_IO_PORT_GET_VALUE,           $// Get port value$
  PLATFORM_IO_PORT_DIR_INPUT,           $// Set port as input$
  PLATFORM_IO_PORT_DIR_OUTPUT           $// Set port as output$
}; ]],
      name = "PIO operations",
      desc = [[These are the operations that can be executed by the PIO subsystem on both ports and pins. They are given as arguments to the @#platform_pio_op@platform_pio_op@ function 
  shown below.]]
    },

    { text = "typedef u32 pio_type;",
      name = "PIO data type",
      desc = [[This is the type used for the actual I/O operations. Currently defined as an unsigned 32-bit type, thus no port can have more than 32 pins. If this happens, it is possible to split 
  it in two or more parts and adding the new parts as "virtual ports" (logical ports that don't have a direct hardware equivalent). The "virtual port" technique is used in the AVR32 backend.]]
    }
  },

  -- Functions
  funcs = 
  {
    { sig = "int #platform_pio_has_port#( unsigned port );",
      desc = [[Checks if the platform has the hardware port specified as argument. Implemented in %src/common.c%, it uses the $NUM_PIO$ macro that must be defined in the
  platform's $cpu_xxx.h$ file (see @arch_overview.html#platforms@here@ for details). For example:</p>
  ~#define NUM_PIO   4      $// The platform has 4 hardware PIO ports$~<p> ]],
      args = "$port$ - the port ID",
      ret = "1 if the port exists, 0 otherwise",
    },

    { sig = "int #platform_pio_has_pin#( unsigned port, unsigned pin );",
      desc = [[Checks if the platform has the hardware port and pin specified as arguments. Implemented in %src/common.c%, it uses the $NUM_PIO$ macro to check the validity
  of the port and the $PIO_PINS_PER_PORT$ or $PIO_PIN_ARRAY$ macros to check the validity of the pin. The macros must be defined in the platform's $platform_conf.h$ file
  (see @arch_overview.html#platforms@here@ for details).</p>
  <ul>
    <li>use $PIO_PINS_PER_PORT$ when all the ports of the MCU have the same number of pins. For example:
      ~#define PIO_PINS_PER_PORT    8   $// Each port has 8 pins$~</li>
    <li>use $PIO_PIN_ARRAY$ when different ports of the MCU have different number of pins. For example:
      ~#define PIO_PIN_ARRAY    { 4, 4, 2, 6 } $// Port 0 has 4 pins, port 1 has 4 pins, port 2 has 2 pins, port 3 has 6 pins$~</li>
  </ul><p>]],
      args = 
      {
        "$port$ - the port ID",
        "$pin$ - the pin number"
      },
      ret = "1 if the pin exists, 0 otherwise",
    },

    { sig = "int #platform_pio_get_num_pins#( unsigned port );",
      desc = "Returns the number of pins for the specified port.",
      args = "$port$ - the port number.",
      ret = "The number of pins for the port."
    },

    { sig = "const char* #platform_pio_get_prefix#( unsigned port );",
      desc = [[Get the port prefix. Used to establish if the port notation uses numbers (P0, P1, P2...) or letters (PA, PB, PC...). Implemented in %src/common.c%, it uses the 
  $PIO_PREFIX$ macro that must be defined in the platform's $platform_conf.h$ file (see @arch_overview.html#platforms@here@ for details). The value of this macro can be either '0' (for
  numeric notation) or 'A' (for letter notation). For example:</p>
  ~#define PIO_PREFIX    'A'   $// Use PA, PB, PC ... for port notation$~<p>]],
      args = "$port$ - the port ID",
      ret = "the port prefix (either '0' or 'A')",  
    },

    { sig = "pio_type #platform_pio_op#( unsigned port, pio_type pinmask, int op );",
      link = "platform_pio_op",
      desc = "This is the function that does the actual I/O work. It is implemented in the platform's own porting layer (%platform.c%, see @arch_overview.html#ports@here@ for more details).",
      args = 
      {
        "$port$ - the port number",
        [[$pinmask$ - has different meanings:
  <ul>
    <li>for $pin operations$ it is the mask of pins in the operation. Each pin on which the function action is executed is encoded with an 1 in the corresponding bit position 
        of the pinmask.</li>
    <li>for $port operations$ it is only meaningful for $PLATFORM_IO_PORT_SET_VALUE$ and in this case it specifies the new value of the port.</li>
  </ul>]],
       "$op$ - specifies the I/O operations, as specified @#pio_operations@here@."
      },
     ret = 
     {
       "an actual value for $PLATFORM_IO_PIN_GET$ (0 or 1) and $PLATFORM_IO_PORT_GET$ (the value of the port).",
       [[an error flag for all the other operations: 1 if the operation succeeded, 0 otherwise. For example, a platform that doesn't have pulldowns on its ports will always return a 0
        when called with the $PLATFORM_IO_PIN_PULLDOWN$ operation.]]
     }
    },
  }
}


data_pt = data_en
