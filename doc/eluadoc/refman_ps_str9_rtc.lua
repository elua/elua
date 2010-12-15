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
    { sig = "#str9.rtc.settime#( time )",
      desc = "Sets the Real Time Clock time to a specific time of the day.",
      args = 
      {
        "$time$ - a string in the format 'hh:mm:ss' or a Lua table with 'hour', 'min' and 'sec' string fields.",
      },
      ret = "nothing.",
      ex = 'str9.settime("14:25:00") - Sets the RTC time to 14 hour 25 minutes, 2:25 PM',
    },
    { sig = "#str9.rtc.gettime#( format )",
      desc = "Gets the time kept by the Real Time Clock.",
      args =
      {
        "$format$ - the string '*s' to return the time as a string 'hh:mm:ss' or '*t' to return as a Lua table with string fields 'hour', 'min' and 'sec'.",
      },
      ret =  "a string or a Lua table, according to the format argument.",
      ex = 'now = str9.rtc.gettime( "*s" ) - now receives a sting like "14:25:05", now = str9.rtc.gettime( "*t" ) - now receives the Lua table { hour = 14, min = 25, sec = 05 }',
    },
    { sig = "#str9.rtc.setdate#( date )",
      desc = "Sets the Real Time Clock date to a specific date.",
      args = 
      {
        "$date$ - a string in the format 'dd/mm/yyyy' or a Lua table with 'day', 'month' and 'year' string fields.",
      },
      ret = "nothing.",
      ex = 'str9.rtc.setdate( "31/08/1960" ) - set the RTC date to August 31st 1960',
    },
    { sig = "#str9.rtc.getdate#( format )",
      desc = "Gets the date kept by the Real Time Clock.",
      args = 
      {
        "$format$ - the string '*s' to return the date as a string 'dd/mm/yyyy' or '*t' to return as a Lua table with string fields 'day', 'month' and 'year'.",
      },
      ret = "a string or a Lua table, according to the format argument.",
      ex = 'today = str9.rtc.getdate( "*s" ) - today receives a string like "14/12/2010", meaning December 14th of 2010, today = str9.rtc.getdate( "*t" ) - today receives the Lua table { day = 14, month = 12, year = 2010 }',
    },

  },
}

data_pt = data_en
