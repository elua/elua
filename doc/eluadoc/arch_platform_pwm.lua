-- eLua platform interface - PWM

data_en = 
{
  -- Title
  title = "eLua platform interface - PWM",

  -- Menu name
  menu_name = "PWM",

  -- Overview
  overview = "This part of the platform interface groups functions related to the PWM channel(s) of the MCU.",

  -- Data structures, constants and types
  structures = 
  {
    { text = [[// PWM operations
enum
{
  PLATFORM_PWM_OP_START,
  PLATFORM_PWM_OP_STOP,
  PLATFORM_PWM_OP_SET_CLOCK,
  PLATFORM_PWM_OP_GET_CLOCK
} ]],
      name = "PWM operations",
      desc = "This enum lists all the operations that can be executed on a given PWM channel."
    },
  },

  -- Functions
  funcs = 
  {
    { sig = "int #platform_pwm_exists#( unsigned id );",
      desc = [[Checks if the platform has the PWM channel specified as argument. Implemented in %src/common.c%, it uses the $NUM_PWM$ macro that must be defined in the
  platform's $platform_conf.h$ file (see @arch_overview.html#platforms@here@ for details). For example:</p>
  ~#define NUM_PWM   4      $// The platform has 4 PWM channels$~<p> ]],
      args = "$id$ - PWM channel ID",
      ret = "1 if the specified PWM channel exists, 0 otherwise"
    },

    { sig = "u32 #platform_pwm_setup#( unsigned id, u32 frequency, unsigned duty );",
      desc = "Sets up a PWM channel",
      args = 
      {
        "$id$ - PWM channel ID",
        "$frequency$ - PWM channel frequency (in hertz)",
        "$duty$ - PWM channel duty cycle, specified as percent (from 0 to 100). Note that some platform don't allow the full 0%-100% duty cycle"
      },
      ret = "The actual frequency set on the PWM channel, which might differ from the $frequency$ parameter, depeding on the hardware",
    },

    { sig = "u32 #platform_pwm_start#( unsigned id );",
      desc = "Starts PWM generation on the specified channel",
      args = 
      {
        "$id$ - PWM channel ID",
      }
    },

    { sig = "u32 #platform_pwm_stop#( unsigned id );",
      desc = "Stops PWM generation on the specified channel",
      args = 
      {
        "$id$ - PWM channel ID",
      }
    },

    { sig = "u32 #platform_pwm_set_clock#( unsigned id, u32 clock );",
      desc = [[Sets the $base$ clock of the specified PWM channel
        (which will be used to generate the frequencies requested by 
        @#platform_pwm_setup@platform_pwm_setup@) to $data$ hertz.]],
      args = 
      {
        "$id$ - PWM channel ID",
        "$clock$ - the desired frequency of the base clock."
      },
      ret = [[The actual value of the base clock, which might be different
        from $data$ depending on the hardware]],
    },

    { sig = "u32 #platform_pwm_get_clock#( unsigned id );",
      desc = "Gets the $base$ clock of the specified PWM channel",
      args = 
      {
        "$id$ - PWM channel ID",
        "$clock$ - the desired frequency of the base clock."
      },
      ret = "the value of the base clock"
    },
  }
}

data_pt = data_en
