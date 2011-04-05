-- eLua reference manual - pwm module

data_en = 
{

  -- Title
  title = "eLua reference manual - pwm module",

  -- Menu name
  menu_name = "pwm",

  -- Overview
  overview = [[This module contains functions for accessing the PWM (Pulse Width Modulation) modules of the eLua CPU.]],

  -- Functions
  funcs = 
  {
    { sig = "frequency = #pwm.setup#( id, frequency, duty )",
      desc = "Setup the PWM modules.",
      args = 
      {
        "$id$ - the ID of the PWM module.",
        "$frequency$ - the frequency of the PWM module (in Hz).",
        [[$duty$ - the duty cycle of the PWM module given in percents. This must be an integer between 0 and 100. $NOTE$: depending on the hardware, some
duty cycles (particulary 0 and 100) might not be achievable.]]
      },
      ret = "The actual frequency set on the PWM module. Depending on the hardware, this might have a different value than the $frequency$ argument."
    },

    { sig = "#pwm.start#( id )",
      desc = "Start the PWM signal on the given module.",
      args = "$id$ - the ID of the PWM module."
    },

    { sig = "#pwm.stop#( id )",
      desc = "Stop the PWM signal on the given module.",
      args = "$id$ - the ID of the PWM module."
    },

    { sig = "clock = #pwm.setclock#( id, clock )",
      desc = "Set the base clock of the given PWM module.",
      args = 
      {
        "$id$ - the ID of the PWM module.",
        "$clock$ - the desired base clock."
      },
      ret = "The actual base clock set on the PWM module.  Depending on the hardware, this might have a different value than the $clock$ argument."
    },

    {  sig = "clock = #pwm.getclock#( id )",
       desc = "Get the base clock of the given PWM module.",
       args = "$id$ - the ID of the PWM module.",
       ret = "The base clock of the PWM module."
    }
  },

}

data_pt = data_en
