-- eLua reference manual - CPU module

data_en = 
{

  -- Title
  title = "eLua reference manual - CPU module",

  -- Menu name
  menu_name = "cpu",

  -- Overview
  overview = [[This module deals with low-level access to CPU (and related modules) functionality, such as reading and writing memory, or 
  enabling and disabling interrupts. It also offers access to platform specific CPU-related constants, as explained @#cpu_constants@here@.]],

  -- Data structures, constants and types
  structures = 
  {
    { text = [[cpu.INT_GPIOA
cpu.INT_GPIOB
.............
cpu.INT_UDMA]],
      name = "CPU constants",
      desc = [[eLua has a mechanism that lets the user export an unlimited number of constants to the $cpu$ module. Although in theory any kind of constant can be exposed by this module,
one should only use constants related to the CPU and its subsystems (as shown above, where a number of CPU specific interrupt masks are exposed to Lua using this mechanism). The constants
visible to the CPU module are given by 3 separate macros (each can be empty):</p>
<ul>
  <li> <b>PLATFORM_CPU_CONSTANTS_INTS</b>: the IDs of the interrupt supported by the @inthandlers.html@eLua interrupt subsystem@ on this CPU.</li>
  <li> <b>PLATFORM_CPU_CONSTANTS_PLATFORM</b>: various CPU and platform related constants (for example peripheral register addresses and maps).</li>
  <li> <b>PLATFORM_CPU_CONSTANTS_CONFIGURED</b>: these are defined in the @configurator.html@configurator@.</li>
</ul>
<p>To use this mechanism, just declare the above constants your platform's header files (for example the CPU definition file) and list your constants as part of this macro,
each enclosed in a special macro called $_C$. For example, to get the constants listed above declare your $PLATFORM_CPU_CONSTANTS$ macro like this:</p>
~#define PLATFORM_CPU_CONSTANTS_PLATFORM\
  _C( INT_GPIOA ),\
  _C( INT_GPIOB ),\
  .................
  _C( INT_UDMA )~
<p>It's worth to note that adding more constants does not increase RAM usage, only Flash usage, so you can expose as many constants as you need without worrying about RAM consumption.]]
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

    { sig = "#cpu.cli#( [id], [resnum1], [resnum2], ... [resnumn])",
      desc = "Disables the global CPU interrupt flag if called without arguments, or a specific interrupt for a list of resource IDs if called with arguments.",
      args =
      {
        "$id$ - the interrupt ID. If specified, at least one resource ID must also be specified.",
        "$resnum1$ - the first resource ID, required if $id$ is specified.",
        "$resnum2 (optional)$ - the second resource ID.",
        "$resnumn (optional)$ - the #n#-th resource ID."
      }
    },

    { sig = "#cpu.sei#( [id], [resnum1], [resnum2], ... [resnumn])",
      desc = "Enables the global CPU interrupt flag if called without arguments, or a specific interrupt for a list of resource IDs if called with arguments.",
      args =
      {
        "$id$ - the interrupt ID. If specified, at least one resource ID must also be specified.",
        "$resnum1$ - the first resource ID, required if $id$ is specified.",
        "$resnum2 (optional)$ - the second resource ID.",
        "$resnumn (optional)$ - the #n#-th resource ID."
      }     
    },
    
    { sig = "clock = #cpu.clock#()",
      desc = "Get the CPU core frequency.",
      ret = "$clock$ - the CPU clock (in Hertz)."
    },

    { sig = "prev_handler = #cpu.set_int_handler#( id, handler )",
      desc = "Sets the Lua interrupt handler for interrupt *id* to function *handler*. *handler* can be #nil# to disable the interrupt handler. Only available if interrupt support is enabled, check @inthandlers.html@here@ for details.",
      args = 
      {
        "$id$ - the interrup ID.",
        "$handler$ - the Lua interrupt handler function, or *nil* to disable the Lua interrupt handler feature."
      },
      ret = "$prev_handler$ - the previous interrupt handler for interrupt *id*, or *nil* if an interrupt handler was not set for interrupt *id*."
    },

     { sig = "handler = #cpu.get_int_handler#( id )",
      desc = "Returns the Lua interrupt handler for interrupt *id*",
      args = "$id$ - the interrup ID.",
      ret = "$handler$ - the interrupt handler for interrupt *id*, or *nil* if an interrupt handler is not set for interrupt *id*."
    },

   { sig = "#cpu.get_int_flag#( id, resnum, [clear] )",
      desc = "Get the interrupt pending flag of an interrupt ID/resource ID combination, and optionally clear the pending flag. Only available if interrupt support is enabled, check @inthandlers.html@here@ for details.",
      args = 
      {
        "$id$ - the interrupt ID.",
        "$resnum$ - the resource ID.",
        "$clear (optional)$ - $true$ to clear the interrupt pending flag or $false$ to leave the interrupt pending flag untouched. Defaults to $true$ if not specified."
      }
    }
  }
}

