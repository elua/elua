-- eLua reference manual - str9 platform specific rtc - Real Time Clock - data

data_en = 
{

  -- Title
  title = "eLua reference manual - STR9 rtc module",

  -- Menu name
  menu_name = "rtc",

  -- Overview
  overview = [[This module contains functions for accesing the particular features of the RTC - Real Time Clock - subsystem of the STR9 family of CPUs.
 This internal subsystem offers functions to keep track of a real time clock calendar, as well as some other features like alarms and auxiliar functions.
  Reference manual, available from ST at @http://www.st.com/mcu/devicedocs-STR912FAW44-101.html@this address@.]],

  -- Functions
  funcs = 
  {
    { sig = "#str9.pio.settime#( time )",
      desc = "Sets the Real Time Clock time to a specific time of the day.",
      args = 
      {
        "$time$ - a string in the format 'hh:mm:ss' or a Lua table with 'hour', 'minute' and 'second' string fields.",
      },
      ret = "nothing."
    },
    { sig = "#str9.pio.gettime#( format )",
      desc = "Gets the time kept by the Real Time Clock.",
      args =
      {
        "$format$ - the string '*s' to return the time as a string 'hh:mm:ss' or '*t' to return as a Lua table with string fields 'hour', 'minute' and 'second'.",
      },
      ret =  "a string or a Lua table, according to the format argument."
    },
    { sig = "#str9.pio.setdate#( date )",
      desc = "Sets the Real Time Clock date to a specific date.",
      args = 
      {
        "$date$ - a string in the format 'dd/mm/yyyy' or a Lua table with 'day', 'month' and 'year' string fields.",
      },
      ret = "nothing."
    },
    { sig = "#str9.pio.getdate#( format )",
      desc = "Gets the date kept by the Real Time Clock.",
      args = 
      {
        "$format$ - the string '*s' to return the date as a string 'dd/mm/yyyy' or '*t' to return as a Lua table with string fields 'day', 'month' and 'year'.",
      },
      ret = "a string or a Lua table, according to the format argument."
    },

  },
}

data_pt = data_en
