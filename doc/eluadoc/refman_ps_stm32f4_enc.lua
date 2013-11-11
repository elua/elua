-- eLua reference manual - platform data

data_en = 
{

  -- Title
  title = "eLua reference manual - STM32F4 enc module",

  -- Menu name
  menu_name = "enc",

  -- Overview
  overview = [[This module contains functions to set up and use timers in quadrature encoder mode.]],

  -- Functions
  funcs = 
  {
    { sig = "#stm32f4.enc.init#( id )",
      desc = "Configure the timer in quadrature mode.",
      args = 
      {
        "$id$ - the timer ID"
      }
    },
    { sig = "#stm32f4.enc.setcounter#( id, count )",
      desc = "Set the current count on a timer.",
      args = 
      {
        "$id$ - the timer ID",
        "$count$ - value to set counter to"
      }
    },
    { sig = "#stm32f4.enc.setidxtrig#( id, resnum, tmr_id, count )",
      desc = "Set up a trigger to set the counter to a specific value when an interrupt is triggered",
      args = 
      {
        "$id$ - the interrupt ID",
        "$resnum$ - resource ID associated with interrupt",
        "$tmr_id$ - the timer ID",
        "$count$ - value to set counter to when triggered"
      }
    },

  },
}

data_pt = data_en
