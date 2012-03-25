-- eLua reference manual - platform data

data_en = 
{

  -- Title
  title = "eLua reference manual - Mizar32 Real Time Clock module",

  -- Menu name
  menu_name = "rtc",

  -- Overview
  overview = [[This module contains functions to drive the Real-Time Clock present on the Mizar32's ethernet module.</p>
<p>The functions accept and return the time and date as a table in the same format at Lua's $os.date()$ and $os.time()$ functions, with fields $year$ (1900-2099), $month$ (1-12), $day$ (1-31), $wday$ (1-7), $hour$ (0-23), $min$ (0-59) and $sec$ (0-59). For $wday$, the day of the week, by convention, 1 is Sunday.]],

  -- Functions
  funcs = 
  {
    { sig = "#mizar32.rtc.set#(datetime)",
      desc = "Sets the time and/or date into the RTC hardware.",
      args = [[$datetime$ - A table with the time and/or date fields to set. Any fields which are not present (or are $nil$) are not changed, so you can set the time, the date or individual fields separately. Setting nonexistent dates like the 31st of February is possible but what will happen next is unpredictable.]],
    },

    { sig = "datetime = #mizar32.rtc.get#()",
      desc = "Reads the current date and time from the RTC hardware.",
      ret = "$datetime$ - A table with fields $year$, $month$, $day$, $wday$, $hour$, $min$ and $sec$.",
    },
  },
}

data_pt = data_en
