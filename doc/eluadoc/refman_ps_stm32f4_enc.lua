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
      desc = "Configure timer for quadrature mode.",
      args = 
      {
        "$id$ - timer ID"
      }
    },
    { sig = "#stm32f4.enc.setcounter#( id, count )",
      desc = "Set the counter for a timer.",
      args = 
      {
        "$id$ - timer ID",
        "$count$ - value to set counter to"
      }
    },
    { sig = "#stm32f4.enc.setidxtrig#( id, resnum, tmr_id, count )",
      desc = "Configure an interrupt trigger to set timer counter to a predefined value.",
      args = 
      {
        "$id$ - interrupt ID",
        "$resnum$ - resource ID associated with interrupt",
        "$tmr_id$ - timer ID",
        "$count$ - value to set counter to when triggered"
      }
    },

  },
}

data_pt = data_en
