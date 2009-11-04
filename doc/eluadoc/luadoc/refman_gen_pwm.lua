-- eLua reference manual - platform data

data_en = 
{

  -- Title
  title = "eLua reference manual - PWM",

  -- Menu name
  menu_name = "pwm",

  -- Overview
  overview = [[This module contains functions that control pulse-width modulation (PWM) peripherals.]],

  -- Functions
  funcs = 
  {
	  { sig = "realfrequency = #pwm.setup#( id, frequency, duty )",
      desc = "Configure PWM channel.",
      args = 
      {
        "$id$ - PWM channel ID.",
				"$frequency$ - Frequency of PWM channel cycle (in hertz).",
        "$duty$ - PWM channel duty cycle, specified as percent (from 0 to 100). Note that some platform don't allow the full 0-100 duty cycle"
      },
      ret = "$realfrequency$ - actual PWM cycle frequency"
    },
		{ sig = "#pwm.start#( id )",
      desc = "Start PWM waveform generation.",
      args = 
      {
        "$id$ - PWM channel ID.",
      }
    },
		{ sig = "#pwm.stop#( id )",
      desc = "Stop PWM waveform generation.",
      args = 
      {
        "$id$ - PWM channel ID.",
      }
    },
	  { sig = "realfrequency = #pwm.setclock#( id, clock )",
      desc = "Set base PWM clock frequency",
      args = 
      {
        "$id$ - PWM channel ID.",
				"$clock$ - Frequency of base clock.",
      },
      ret = "$realfrequency$ - actual base PWM clock."
    },
	  { sig = "clock = #pwm.getclock#( id )",
      desc = "Get base PWM clock frequency",
      args = 
      {
        "$id$ - PWM channel ID.",
      },
      ret = "$clock$ - base PWM clock."
    },
  },
}