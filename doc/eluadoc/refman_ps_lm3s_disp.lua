-- eLua reference manual - platform data

data_en = 
{

  -- Title
  title = "eLua reference manual - LM3S disp module",

  -- Menu name
  menu_name = "disp",

  -- Overview
  overview = [[This module contains functions for working with the RIT OLED display on the Luminary Micro EKx-LM3S8962 boards and others.]],

  -- Functions
  funcs = 
  {
    { sig = "#lm3s.disp.init#( frequency )",
      desc = "Initialize the display.",
      args = "$frequency$ - Number, the clock frequency (in Hertz) of the SSI interface used to control the display."
    },

    { sig = "#lm3s.disp.enable#( frequency )",
      desc = "Enable the display.",
      args = "$frequency$ - Number, the clock frequency (in Hertz) of the SSI interface used to control the display."     
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
      desc = "Clear the display."
    },

    { sig = "#lm3s.disp.print#( str, x, y, col )",
      desc = "Write a string on the display. A 5x7 font (in a 6x8 cell) is used for drawing the text.",
      args = 
      {
        "$str$ - String, the text to be written on the display.",
        "$x$ - Number [0-127], the horizonal position of the text (specified in columns).",
        "$y$ - Number [0-95], the vertical position of the text (specified in lines).",
        "$col$ - Number [0-15], the 4-bit gray scale value to be used for the text."
      }
    },

    { sig = "#lm3s.disp.draw#( img, x, y, width, height )",
      desc = "Draw an image on the display.",
      args = 
      {
        "$img$ - String, the image to draw in string format.",
        "$x$ - Number [0-127], the horizontal position of the image (specified in pixels).",
        "$y$ - Number [0-95], the vertical position of the image (specified in pixels).",
        "$width$ - Number [1-127], the width of the image.",
        "$height$ - Number [1-95], the height of the image."
      }
    },
    
  },
}

data_pt = data_en
