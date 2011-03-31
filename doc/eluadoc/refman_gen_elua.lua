-- eLua reference manual - elua module

data_en = 
{

  -- Title
  title = "eLua reference manual - elua module",

  -- Menu name
  menu_name = "elua",

  -- Overview
  overview = [[This module is an interface to the core system and services of eLua. Some functions from this module can change the runtime behaviour of eLua, which in turn can lead to instability or unexpected program behaviour. $Use this module with care$.]],

  -- Functions
  funcs = 
  {
    { sig = "#elua.egc_setup#( mode, [memlimit] )",
      desc = "Change the emergency garbage collector operation mode and memory limit (see @elua_egc.html@here@ for details).",
      args = 
      {
        "$mode$ - the EGC operation mode. Can be either $elua.EGC_NOT_ACTIVE$, $elua.EGC_ON_ALLOC_FAILURE$, $elua.EGC_ON_MEM_LIMIT$, $elua.EGC_ALWAYS$ or a combination between the last 3 modes in this list (they can be combined both with bitwise OR operations, using the @refman_gen_bit.html@bit@ module, or simply by adding them).",
        "$memlimit$ - required only when $elua.EGC_ON_MEM_LIMIT$ is specified in $mode$, specifies the EGC upper memory limit."
      },
    },
    
    { sig = "#elua.save_history#( filename )",
      desc = "Save the interpreter line history. Only available if linenoise is enabled, check @linenoise.html@here@ for details.",
      args = "$filename$ - the name of the file where the history will be saved. $CAUTION$: the file will be overwritten.",
    },    

    { sig = "version = #elua.version#()",
      desc = "Returns the current eLua version as a string",
      ret = "the eLua version currently running."
    },
  },
}

data_pt = data_en
