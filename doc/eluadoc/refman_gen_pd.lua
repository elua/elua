-- eLua reference manual - platform data

data_en = 
{

  -- Title
  title = "eLua reference manual - platform data",

  -- Menu name
  menu_name = "pd",

  -- Overview
  overview = [[This module contains functions that access specific platform data. Useful if the code needs to know on which platform it runs.]],

  -- Functions
  funcs = 
  {
    { sig = "platform = #pd.platform#()",
      desc = "Get platform name.",
      ret = "$platform$ - the name of the platform on which eLua is running.",
    },

    { sig = "cpu = #pd.cpu#()",
      desc = "Get CPU name.",
      ret = "$cpu$ - the name of the CPU of the platform on which eLua is running.",
    },

    { sig = "board = #pd.board#()",
      desc = "Get board name.",
      ret = "$board$ - the name of the board on which eLua is running.",
    }
  },
}

data_pt = data_en
