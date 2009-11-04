-- eLua reference manual - platform data

data_en = 
{

  -- Title
  title = "eLua reference manual - Timer module",

  -- Menu name
  menu_name = "tmr",

  -- Overview
  overview = [[This module contains functions that control timer peripherals.]],

  -- Functions
  funcs = 
  {
	  { sig = "#tmr.delay#( id, period )",
      desc = "Block for a period of time (in microseconds)",
      args = 
      {
        "$id$ - Timer channel ID.",
				"$period$ - the delay time (in microseconds)"
      },
    },
	  { sig = "timervalue = #tmr.read#( id )",
      desc = "Get the value of the specified timer's counter register.",
      args = 
      {
        "$id$ - Timer channel ID.",
      },
      ret = "$timervalue$ - the timer's counter register"
    },
	  { sig = "timervalue = #tmr.start#( id )",
      desc = "Start the specified timer by setting its counter register to a predefined value.",
      args = 
      {
        "$id$ - Timer channel ID.",
      },
      ret = "$timervalue$ - the timer's counter register"
    },
	  { sig = "time_us = #tmr.diff#( id, end, start )",
      desc = "Return the time difference (in us) betweeen two timer values.",
      args = 
      {
        "$id$ - Timer channel ID.",
				"$end$ - the first timer value",
				"$start$ - the second timer value"
      },
      ret = "$time_us$ - the time difference (in microseconds)"
    },
	  { sig = "mindelay_us = #tmr.mindelay#( id )",
      desc = "Get the minimum achievable timeout on the specified timer (in us).",
      args = 
      {
        "$id$ - Timer channel ID.",
      },
      ret = "$mindelay_us$ - the minimum achievable delay (in microseconds)"
    },
	  { sig = "maxdelay_us = #tmr.maxdelay#( id )",
      desc = "Get the maximum achievable timeout on the specified timer (in us).",
      args = 
      {
        "$id$ - Timer channel ID.",
      },
      ret = "$maxdelay_us$ - the maximum achievable delay (in microseconds)"
    },
	  { sig = "realclock = #tmr.setclock#( id, clock )",
      desc = "Set the clock of the specified timer.",
      args = 
      {
        "$id$ - Timer channel ID.",
				"$clock$ - desired clock (in hertz)"
      },
      ret = "$realclock$ - actual clock set on the timer"
    },
	  { sig = "clock = #tmr.getclock#( id )",
      desc = "Get the clock of the specified timer.",
      args = 
      {
        "$id$ - Timer channel ID."
      },
      ret = "$clock$ - timer clock"
    },
  },
}