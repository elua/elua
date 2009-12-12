-- eLua reference manual - CPU module

data_en = 
{

  -- Title
  title = "eLua reference manual - CPU module",

  -- Menu name
  menu_name = "cpu",

  -- Overview
  overview = [[This module deals with low-level access to CPU (and related modules) functionality, such as reading and writing memory, or 
  enabling and disabling interrupts. It also offers access to platform specific CPU-related constants using a special macro defined in the
  platform's $platform_conf.h$ file, as explained @#cpu_constants@here@.]],

  -- Data structures, constants and types
  structures = 
  {
    { text = [[cpu.INT_GPIOA
cpu.INT_GPIOB
.............
cpu.INT_UDMA]],
      name = "CPU constants",
      desc = [[eLua has a mechanism that lets the user export an unlimited number of constants to the $cpu$ module. Although in theory any kind of constant can be exposed by this module,
one should only use constants related to the CPU and its subsystems (as shown above, where a number of CPU specific interrupt masks are exposed to Lua using this mechanism). To use this
mechanism, just declare the $PLATFORM_CPU_CONSTANTS$ macro in your platform's $platform_conf.h$ file and list all your constants as part of this macro, each enclosed in a special macro called
$_C$. For example, to get the constants listed above declare your $PLATFORM_CPU_CONSTANTS$ macro like this:</p>
~#define PLATFORM_CPU_CONSTANTS\
  _C( INT_GPIOA ),\
  _C( INT_GPIOB ),\
  .................
  _C( INT_UDMA )~
<p>It's worth to note that adding more constants does not increas RAM usage, only Flash usage, so you can expose as much constants as you need without worrying about RAM consumption.]]
    },
  },

  -- Functions
  funcs = 
  {
    { sig = "#cpu.w32#( address, data )",
      desc = "Writes a 32-bit word to memory.",
      args = 
      {
        "$address$ - the memory address.",
        "$data$ - the 32-bit data to write."
      },
    },

    { sig = "data = #cpu.r32#( address )",
      desc = "Read a 32-bit word from memory.",
      args = "$address$ - the memory address.",
      ret = "$data$ - the 32-bit word read from memory."
    },

    { sig = "#cpu.w16#( address, data )",
      desc = "Writes a 16-bit word to memory.",
      args =
      {
        "$address$ - the memory address.",
        "$data$ - the 16-bit data to write."
      },
    },

    { sig = "data = #cpu.r16#( address )",
      desc = "Reads a 16-bit word from memory.",
      args = "$address$ - the memory address.",
      ret = "$data$ - the 16-bit word read from memory."
    },

    { sig = "#cpu.w8#( address, data )",
      desc = "Writes a byte to memory.",
      args =
      {
        "$address$ - the memory address.",
        "$data$ - the byte to write."
      }
    },

    { sig = "data = #cpu.r8#( address )",
      desc = "Reads a byte from memory.",
      args = "$address$ - the memory address",
      ret = "$data$ - the byte read from memory."
    },

    { sig = "#cpu.cli#()",
      desc = "Disable CPU interrupts."
    },

    { sig = "#cpu.sei#()",
      desc = "Enable CPU interrupts."
    },

    { sig = "clock = #cpu.clock#()",
      desc = "Get the CPU core frequency.",
      ret = "$clock$ - the CPU clock (in Hertz)."
    }
  },
}

data_pt = 
{

  -- Title
  title = "eLua reference manual - CPU module",

  -- Menu name
  menu_name = "cpu",

  -- Overview
  overview = [[This module deals with low-level access to CPU (and related modules) functionality, such as reading and writing memory, or 
  enabling and disabling interrupts. It also offers access to platform specific CPU-related constants using a special macro defined in the
  platform's $platform_conf.h$ file, as exaplained @#cpu_constants@here@.]],

  -- Data structures, constants and types
  structures = 
  {
    { text = [[cpu.INT_GPIOA
cpu.INT_GPIOB
.............
cpu.INT_UDMA]],
      name = "CPU constants",
      desc = [[eLua has a mechanism that lets the user export an unlimited number of constants to the $cpu$ module. Although in theory any kind of constant can be exposed by this module,
one should only use constants related to the CPU and its subsystems (as shown above, where a number of CPU specific interrupt masks are exposed to Lua using this mechanism). To use this
mechanism, just declare the $PLATFORM_CPU_CONSTANTS$ macro in your platform's $platform_conf.h$ file and list all your constants as part of this macro, each enclosed in a special macro called
$_C$. For example, to get the constants listed above declare your $PLATFORM_CPU_CONSTANTS$ macro like this:</p>
~#define PLATFORM_CPU_CONSTANTS\
  _C( INT_GPIOA ),\
  _C( INT_GPIOB ),\
  .................
  _C( INT_UDMA )~
<p>It's worth to note that adding more constants does not increas RAM usage, only Flash usage, so you can expose as much constants as you need without worrying about RAM consumption.]]
    },
  },

  -- Functions
  funcs = 
  {
    { sig = "#cpu.w32#( address, data )",
      desc = "Writes a 32-bit word to memory.",
      args = 
      {
        "$address$ - the memory address.",
        "$data$ - the 32-bit data to write."
      },
    },

    { sig = "data = #cpu.r32#( address )",
      desc = "Read a 32-bit word from memory.",
      args = "$address$ - the memory address.",
      ret = "$data$ - the 32-bit word read from memory."
    },

    { sig = "#cpu.w16#( address, data )",
      desc = "Writes a 16-bit word to memory.",
      args =
      {
        "$address$ - the memory address.",
        "$data$ - the 16-bit data to write."
      },
    },

    { sig = "data = #cpu.r16#( address )",
      desc = "Reads a 16-bit word from memory.",
      args = "$address$ - the memory address.",
      ret = "$data$ - the 16-bit word read from memory."
    },

    { sig = "#cpu.w8#( address, data )",
      desc = "Writes a byte to memory.",
      args =
      {
        "$address$ - the memory address.",
        "$data$ - the byte to write."
      }
    },

    { sig = "data = #cpu.r8#( address )",
      desc = "Reads a byte from memory.",
      args = "$address$ - the memory address",
      ret = "$data$ - the byte read from memory."
    },

    { sig = "#cpu.cli#()",
      desc = "Disable CPU interrupts."
    },

    { sig = "#cpu.sei#()",
      desc = "Enable CPU interrupts."
    },

    { sig = "clock = #cpu.clock#()",
      desc = "Get the CPU core frequency.",
      ret = "$clock$ - the CPU clock (in Hertz)."
    }
  },
}

