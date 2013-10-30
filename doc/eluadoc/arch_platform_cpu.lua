-- eLua platform interface - CPU

data_en = 
{
  -- Title
  title = "eLua platform interface - CPU",

  -- Menu name
  menu_name = "CPU",

  -- Overview
  overview = "This part of the platform interface groups functions related to the CPU and its functional modules (interrupt controller, memory controller and others).",

  -- Functions
  funcs = 
  {
    { sig = "int #platform_cpu_set_global_interrupts#( int status );",
      desc = "Set or clear the global interrupt flag of the CPU.",
      args = "$status$ - $PLATFORM_CPU_ENABLE$ to set the global interrupt flag or $PLATFORM_CPU_DISABLE$ to clear the global interrupt flag.",
      ret = "the previous value of the global interrupt flag (1 if set, 0 otherwise)."
    },

    { sig = "int #platform_cpu_get_global_interrupts#();",
      desc = "Get the value of the global interrupt flag of the CPU.",
      ret = "the value of the global interrupt flag (1 if set, 0 otherwise)."
    },

    { sig = "int #platform_cpu_set_interrupt#( elua_int_id id, elua_int_resnum resnum, int status );",
      desc = "Enable or disable a specific CPU interrupt for a given resource ID.",
      args = 
      {
        "$id$ - the interrupt ID, as defined in %platform_ints.h%.",
        "$resnum$ - the resource ID.",
        "$status$ - $PLATFORM_CPU_ENABLE to enable the interrupt or $PLATFORM_CPU_DISABLE$ to disable the interrupt.",
      },
      ret = 
      {
        "$PLATFORM_INT_INVALID$ - invalid interrupt ID",
        "$PLATFORM_INT_NOT_HANDLED$ - this interrupt cannot be enabled/disabled",
        "$PLATFORM_INT_BAD_RESNUM$ - this resource ID can't be used to enable/disable the interrupt",
        "the previous status of the interrupt (1 if enabled, 0 otherwise) if no error occured."
      }
    },

    { sig = "int #platform_cpu_get_interrupt#( elua_int_id id, elua_int_resnum resnum );",
      desc = "Get the interrupt enabled status of a specific CPU interrupt for a given resource ID.",
      args = 
      {
        "$id$ - the interrupt ID, as defined in %platform_ints.h%",
        "$resnum$ - the resource ID"
      },
      ret =
      {
        "$PLATFORM_INT_INVALID$ - invalid interrupt ID",
        "$PLATFORM_INT_NOT_HANDLED$ - this interrupt cannot be enabled/disabled",
        "$PLATFORM_INT_BAD_RESNUM$ - this resource ID can't be used to enable/disable the interrupt",
        "the status of the interrupt (1 if enabled, 0 otherwise) if no error occured."
      }
    },

    { sig = "int #platform_cpu_get_interrupt_flag#( elua_int_id id, elua_int_resnum resnum, int clear );",
      desc = "Return the interrupt pending flag of a specific CPU interrupt or a given resource ID and optionally clear it.",
      args = 
      {
        "$id$ - the interrupt ID, as defined in %platform_ints.h%.",
        "$resnum$ - the resource ID.",
        "$clear$ - 1 to clear the interrupt pending flag if it is set, 0 otherwise."
      },
      ret = 
      {
        "$PLATFORM_INT_INVALID$ - invalid interrupt ID.",
        "$PLATFORM_INT_NOT_HANDLED$ - this interrupt's flag cannot be read.",
        "$PLATFORM_INT_BAD_RESNUM$ - this resource ID can't be used to read this interrupt's flag.",
        "the value of the interrupt pending flag if no error occured."
      }
    },

    { sig = "u32 #platform_cpu_get_frequency#();",
      desc = "Get the CPU frequency.",
      ret = "the CPU $core$ frequency (in Hertz)."
    },
  }
}

