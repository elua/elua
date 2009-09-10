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
    { sig = "void #platform_cpu_enable_interrupts#();",
      desc = "Enable global interrupt on the CPU."
    },

    { sig = "void #platform_cpu_disable_interrupts#();",
      desc = "Disable global interrupts on the CPU."
    },

    { sig = "u32 #platform_cpu_get_frequency#();",
      desc = "Get the CPU frequency.",
      ret = "the CPU $core$ frequency (in Hertz)."
    },
  }
}

data_pt = 
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
    { sig = "void #platform_cpu_enable_interrupts#();",
      desc = "Enable global interrupt on the CPU."
    },

    { sig = "void #platform_cpu_disable_interrupts#();",
      desc = "Disable global interrupts on the CPU."
    },

    { sig = "u32 #platform_cpu_get_frequency#();",
      desc = "Get the CPU frequency.",
      ret = "the CPU $core$ frequency (in Hertz)."
    },
  }
}
