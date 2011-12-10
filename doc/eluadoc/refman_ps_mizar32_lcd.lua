-- eLua reference manual - platform data

data_en = 
{

  -- Title
  title = "eLua reference manual - Mizar32 LCD module",

  -- Menu name
  menu_name = "lcd",

  -- Overview
  overview = [[This module contains functions to drive the two-line character LCD panel of the Mizar32 display module.</p>
<p>Physically, the display has 16 characters per line but internally it has a 40 character by two line memory. It displays 16 of those 40 columns at a time, with various ways to determine which of the 40 columns appear in the 16-column display. If you just want to display 16x2 characters, the $reset$, $goto$ and $print$ functions are enough to do this.]],

  -- Functions
  funcs = 
  {
    { sig = "#mizar32.lcd.reset#()",
      desc = "Initialises the display, resetting everything to as initial state: clear screen, no cursor, displaying columns 1-16 of the 40-column memory, ready to print at (1,1), writing text from left to right and moving the cursor one place right after each character. You don't %have% to call $reset$ at the start of your program, but doing so does will ensure that your program still works if the display has been left in a funny state by some previous run."
    },

    { sig = "#mizar32.lcd.setup#( display_shift, right_to_left )",
      desc = "This can be used to set some of the stranger operating modes of the LCD display. Both parameters are optional and if you omit them, they default to $false$, which sets sensible mode.",
      args = 
      {
        [[$display_shift$ - If $true$, then with each character you subsequently print, the cursor will move by one place in the character memory as usual but the display's contents will also move by one position horizontally in the opposite direction so that the cursor remains in the same column of the physical display. This can be used to achieve "scrolling text" effects. Note, however, that when the cursor passes from column 40 to column 1 or vice versa, it flips over to the other row.]],
        "$right_to_left$ - If $true$, text will be printed right-to-left: the cursor will move one position to the left in the character memory and, if display shifting is also enabled, the contents of the display will shift to the right so that the cursor stays in the same column on the screen."
      }
    },

    { sig = "#mizar32.lcd.clear#()",
      desc = "Clears the display, moves the cursor to the top left (position 1,1) and resets the display shift to show columns 1 to 16."
    },

    { sig = "#mizar32.lcd.home#()",
      desc = "Moves the cursor to the top left (position 1,1) and resets the display shift."
    },

    { sig = "#mizar32.lcd.goto#( row, column )",
      desc = "Moves the cursor to the specified row and column.",
      args = 
      {
        "$row$ - A number (1 or 2) giving the row you want to move to.",
        "$column$ - A number (1 to 40) giving the position within that row in the character memory."
      }
    },

    { sig = "#row, column = mizar32.lcd.getpos#()",
      desc = "Returns the current cursor position.",
      ret = 
      {
        "$row$ - A number (1 or 2) giving the current row.",
        "$column$ - A number (1 to 40) giving the current column in the character memory."
      }
    },

    { sig = "#mizar32.lcd.print#( [data1] [, data2] ... [datan] )",
      desc = "Writes into the LCD character memory starting at the current cursor position. The cursor will advance by one position for each character printed. When it goes past column 40, it moves to column 1 of the other line, (and vice versa when printing right-to-left).",
      args = 
      {
        "$data$ - Each item of data can be a string or an integer. Strings are the normal way to display messages of ASCII text. An integer parameter should have a value from 0 to 255 to display a single character, which can be one of the user-defined characters 0-7, the regular ASCII characters 32-125 plus 126 and 127 for right- and left-pointing arrows and the chinese, greek and mathematical symbols with codes 160-255."
      }
    },

    { sig = "#mizar32.lcd.cursor#( what )",
      desc = "Sets the type of cursor that is displayed at the cursor position or move the cursor left or right.",
      args = 
      {
        [[$what$ - A string to say what should be done:
          <p>$"none"$, $"line"$ or $"block"$ will display, respectively, no visible cursor, a constant underline or a blinking solid block at the cursor position.
          <p>$"left"$ or $"right"$ move the cursor one position left or right in the character memory and on the display without changing the underlying characters. The display never shifts in this case and, as usual, the cursor wraps between column 40 of one row and column 1 of the other.]]
      }
    },

    { sig = "#mizar32.lcd.display#( what )",
      desc = "Turns the physical display on or off, or shifts the displayed characters left or right.",
      args = 
      {
        [[$what$ - A string to say what should be done:
          <p>$"off"$ and $"on"$ turn the physical display off or back on again. While the display is off it appears blank but the contents of the character memory, the position and type of cursor, user-defined characters and setup mode are all remembered and you can write to the character memory and perform all other operations while the display is off. This allows you to update the display without the viewer seeing too much flickering.
        <p>$"left"$ or $"right"$ shift the displayed characters one place left or right. For example, if it was displaying the usual columns 1-16 and you say %mizar32.lcd.display("left")%, it will then display columns 2-17: the visible characters move left but the window onto the character memory moves right. ]],
      }
    },

    { sig = "#mizar32.lcd.definechar#( code, glyph )",
      desc = "Programs one of the eight user-definable characters whose codes are 0 to 7. When it has been defined, a character can be displayed using $mizar32.lcd.print(n)$, where $n$ is a number from 0 to 7. If the character in question is already being displayed, its visible form will change immediately on the display. At power-on, the 8 characters are defined as random garbage.",
      args = 
      {
        "$code$ - A number (0 to 7) saying which of the characters you wish to redefine.",
        "$glyph$ - A table of up to eight numbers giving the bit-patterns for the eight rows of the character, in order from top to bottom. Each of these number is a value from 0 to 31, to define which of the 5 bits in the row should be black. The pixels' values from left to right are 16, 8, 4, 2 and 1. For example, { 1, 3, 7, 15, 31, 15, 7, 3, 1, 0 } would define a left-pointing solid triangle in the top 7 rows. Extra rows are ignored, and missing rows are blanked."
      }
    },

    { sig = "#buttons = mizar32.lcd.buttons#()",
      desc = "Tells which of the five user buttons are currently pressed.",
      ret = 
      {
        "$buttons$ - A string containing up to five of the characters $L$, $R$, $U$, $D$ and $S$ to say whether the Left, Right, Up, Down and Select buttons are currently held down. If none are pressed, an empty string is returned. The hardare allows Select to be detected reliably and up to two of the other four: if three of Left, Right, Up and Down are being held, all four are returned."
      }
    },
  },
}

data_pt = data_en
