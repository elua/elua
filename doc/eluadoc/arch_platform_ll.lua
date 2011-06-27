-- eLua platform interface - low level functions

data_en = 
{
  -- Title
  title = "eLua platform interface - low level functions",

  -- Menu title
  menu_name = "Low-level",

  -- Overview
  overview = [[
  This part of the platform interface deals contains a small set of "low level functions" that are used to "couple" the eLua port with the
  target system. No eLua module exposes these functions, as they are strictly used for porting and do not provide any other functionality.
  ]],

  -- Data structures, constants and types
  structures = 
  {
    { text = [[// Error / status codes
enum
{
  PLATFORM_ERR,
  PLATFORM_OK,
  PLATFORM_UNDERFLOW = -1
};]],
      name = "Status codes",
      desc = [[
  This enum defines the possible return values of the @#platform_init@platform_init@ function (although only $PLATFORM_ERR$ and $PLATFORM_OK$ should be 
  returned from $platform_init$).
  ]]
    },
  },

  -- Functions
  funcs = 
  {
    { sig = "int #platform_init#();",
      desc = [[This is the platform-specific initialization code. It is the first function called from %main()% ($src/main.c$) and it should handle
  all the platform initialization sequence, included (but not limited to) setting up the proper clocks, initializing the interrupt subsystem,
  setting up various peripherals and so on. Although platform specific, this function has a common part named %cmn_platform_init% (implemented 
  in $src/common.c$) that initializes terminal support over serial connections, as well as the XMODEM and TERM components 
  (see @building.html@here@ for details). If you need any of these, you need to call %cmn_platform_init% at the end of your 
  %platform_init% function, $after$ initializing all the peripherals (in particular the UART used for the serial connection).<br>
  An implementation skeleton for this function is given below:</p>
  ~int platform_init()
  {
    ............. // perform all your initializations here
    cmn_platform_init(); // call the common initialiation code
    return PLATFORM_OK;
  }~<p>]],
      ret = 
      {
         "$PLATFORM_OK$ for success",
         [[$PLATFORM_ERR$ if an error occured. If $PLATFORM_ERR$ is returned,  %main% will block in an infinite loop right 
  after calling this function, so you should return $PLATFORM_ERR$ only for serious errors]],
      }, 
    },

    { sig = "void* #platform_get_first_free_ram#( unsigned id );",
      desc = [[Returns the start address of a free RAM area in the system (this is the RAM that will be used by any part of the code that uses malloc(), 
  a good example being the Lua interpreter itself). There can be multiple free RAM areas in the system (for example the internal MCU RAM and external 
  RAM chips).  Implemented in $src/common.c$, it uses the the $MEM_START_ADDRESS$ macro that must be defined in the platform's $platform_conf.h$
  file (see @arch_overview.html#platforms@here@ for details). This macro must be defined as an array that contains all the start addresses of 
  free RAM in the system. For internal RAM, this is generally handled by a linker exported symbol (named $end$ in many eLua ports) which 
  points to the first RAM address after all the constant and non-constant program data. An example is given below:</p>
  ~#define MEM_START_ADDRESS     { ( void* )end }~<p>]],
      args = "$id$ - the identifier of the RAM area",
      ret = "the start address of the given memory area",   
    },

    { sig = "void* #platform_get_last_free_ram#( unsigned id );",
      desc = [[Returns the last address of a free RAM area in the system (this is the RAM that will be used by any part of the code that uses malloc(), 
  a good example being the Lua interpreter itself). There can be multiple free RAM areas in the system (for example the internal MCU RAM and external 
  RAM chips). Implemented in $src/common.c$, it uses the the $MEM_END_ADDRESS$ macro that must be defined in the platform's $platform_conf.h$
  file (see @arch_overview.html#platforms@here@ for details). This macro must be defined as an array that contains all the end addresses of 
  free RAM in the system. For internal RAM, this is generally set as the last RAM memory address minus the size of the system stack(s). An example is 
  given below:</p>
  ~#define MEM_END_ADDRESS       { ( void* )( SRAM_BASE + 0x10000 - STACK_SIZE_TOTAL - 1 ) }~<p>]],
      args = "$id$ - the identifier of the RAM area",
      ret = "the end address of the given memory area",   
    },

  }
}

data_pt = data_en
