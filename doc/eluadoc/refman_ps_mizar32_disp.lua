-- eLua reference manual - platform data

data_en = 
{

  -- Title
  title = "eLua reference manual - Mizar32 disp module",

  -- Menu name
  menu_name = "disp",

  -- Overview
  overview = [[This module contains functions to drive the 16x2 character LED panel for the Mizar32.]],

  -- Functions
  funcs = 
  {
    { sig = "#mizar32.disp.clear#()",
      desc = "Clear the display and move the cursor to the top left (position 1,1)."
    },

    { sig = "#mizar32.disp.goto#( row, column )",
      desc = "Move the cursor to the specified row and column.",
      args = 
      {
        "$row$ - A number (1 or 2) giving the row you want to move to.",
        "$column$ - A number (1 to 16) giving the character position within that row."
      }
    },

    { sig = "#mizar32.disp.print#( message )",
      desc = "Display characters at the current cursor position, moving the cursor right after each character",
      args = 
      {
        "$message$ - A string of ASCII text to be written on the display."
      }
    },
  },
}

data_pt = data_en
