-- eLua reference manual - platform data

data_en = 
{

  -- Title
  title = "eLua reference manual - LM3S disp module",

  -- Menu name
  menu_name = "disp",

  -- Overview
  overview = [[This module contains functions for working with the OLED display on the EKx-LM3S8962 boards and others.]],

  -- Functions
  funcs = 
  {
    { sig = "#lm3s.disp.init#( frequency )",
      desc = "Initialize the display.",
      args = "$frequency$ - the clock frequency of the SSI interface used to control the display."
    },

    { sig = "#lm3s.disp.enable#( frequency )",
      desc = "Enable the display.",
      args = "$frequency$ - the clock frequency of the SSI interface used to control the display."     
    },

    { sig = "#lm3s.disp.disable#()",
      desc = "Disable the display.",
    },

    { sig = "#lm3s.disp.on#()",
      desc = "Turn the display on."
    },

    { sig = "#lm3s.disp.off#()",
      desc = "Turn the display off."
    },

    { sig = "#lm3s.disp.clear#()",
      desc = "Cleaer the display."
    },

    { sig = "#lm3s.disp.print#( str, x, y, col )",
      desc = "Write a string on the display. A 5x7 font (in a 6x8 cell) is used for drawing the text.",
      args = 
      {
        "$str$ - the string to write.",
        "$x$ - the horizonal position of the text (specified in columns).",
        "$y$ - the vertical position of the text (specified in lines).",
        "$col$ - the 4-bit gray scale value to be used for the text."
      }
    },

    { sig = "#lm3s.disp.draw#( img, x, y, width, height )",
      desc = "Draw an image on the display.",
      args = 
      {
        "$img$ - the image to draw in string format.",
        "$x$ - the horizontal position of the image (specified in pixels).",
        "$y$ - the vertical position of the image (specified in pixels).",
        "$width$ - the width of the image.",
        "$height$ - the height of the image."
      }
    },
    
  },
}

