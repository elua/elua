-- eLua reference manual - term module

data_en = 
{

  -- Title
  title = "eLua reference manual - term module",

  -- Menu name
  menu_name = "term",

  -- Overview
  overview = [[This module contains functions for accessing ANSI-compatible terminals (and terminal emulators) from Lua.]],

  -- Functions
  funcs = 
  {
    { sig = "#term.clrscr#()",
      desc = "Clear the screen",
    },

    { sig = "#term.clreol#()",
      desc = "Clear from the current cursor position to the end of the line",
    },

    { sig = "#term.moveto#( x, y )",
      desc = "Move the cursor to the specified coordinates",
      args=
      {
        "$x$ - the column (starting with 1)",
        "$y$ - the line (starting with 1)"
      }
    },

    { sig = "#term.moveup#( delta )",
      desc = "Move the cursor up",
      args = "$delta$ - number of lines to move the cursor up"
    },

    { sig = "#term.movedown#( delta )",
      desc = "Move the cursor down",
      args = "$delta$ - number of lines to move the cursor down",
    },

    { sig = "#term.moveleft#( delta )",
      desc = "Move the cursor left",
      args = "$delta$ - number of columns to move the cursor left",
    },
 
    { sig = "#term.moveright#( delta )",
      desc = "Move the cursor right",
      args = "$delta$ - number of columns to move the cursor right",
    },

    { sig = "numlines = #term.getlines#()",
      desc = "Get the number of lines in the terminal",
      ret = "The number of lines in the terminal",
    },

    { sig = "numcols = #term.getcols#()",
      desc = "Get the number of columns in the terminal",
      ret = "The number of columns in the terminal",
    },

    { sig = "#term.print#( [ x, y ], str1, [ str2, ..., strn ] )",
      desc = "Write one or more strings in the terminal",
      args = 
      {
        "$x (optional)$ - write the string at this column. If $x$ is specified, $y$ must also be specified",
        "$y (optional)$ - write the string at this line. If $y$ is specified, $x$ must also be specified",
        "$str1$ - the first string to write",
        "$str2 (optional)$ - the second string to write",
        "$strn (optional)$ - the nth string to write"
      }
    },

    { sig = "cx = #term.getcx#()",
      desc = "Get the current column of the cursor",
      ret = "The column of the cursor"
    },

    { sig = "cy = #term.getcy#()",
      desc = "Get the current line of the cursor",
      ret = "The line of the cursor" 
    },

    { sig = "ch = #term.getchar#( [ mode ] )",
      desc = "Read a char (a key press) from the terminal",
      args = [[$mode (optional)$ - terminal input mode. It can be either:</p>
  <ul>
    <li>$term.WAIT$ - wait for a key to be pressed, then return it. This is the default behaviour if $mode$ is not specified. </li>
    <li>$term.NOWAIT$ - if a key was pressed on the terminal return it, otherwise return -1.</li>
  </ul><p>]],
      ret = [[The char read from a terminal or -1 if no char is available. The 'char' can be an actual ASCII char, or a 'pseudo-char' which encodes special keys on
  the keyboard. The list of the special chars and their meaning is given in the table below:</p>
<table style="text-align: left; margin-left: 2em;">
<tbody>
<tr>
  <th style="text-align: left;">Key code</th>
  <th style="text-align: left;">Meaning</th>
</tr>
<tr>
  <td>$KC_UP$</td>
  <td>the UP key on the terminal</td>
</tr>
<tr>
  <td>$KC_DOWN$</td>
  <td>the DOWN key on the terminal</td>
</tr>
<tr>
  <td>$KC_LEFT$</td>
  <td>the LEFT key on the terminal</td>
</tr>
<tr>
  <td>$KC_RIGHT$</td>
  <td>the RIGHT key on the terminal</td>
</tr>
<tr>
  <td>$KC_HOME$</td>
  <td>the HOME key on the terminal</td>
</tr>
<tr>
  <td>$KC_END$</td>
  <td>the END key on the terminal</td>
</tr>
<tr>
  <td>$KC_PAGEUP$</td>
  <td>the PAGE UP key on the terminal</td>
</tr>
<tr>
  <td>$KC_PAGEDOWN$</td>
  <td>the PAGE DOWN key on the terminal</td>
</tr>
<tr>
  <td>$KC_ENTER$</td>
  <td>the ENTER (CR) key on the terminal</td>
</tr>
<tr>
  <td>$KC_TAB$</td>
  <td>the TAB key on the terminal</td>
</tr>
<tr>
  <td>$KC_BACKSPACE$</td>
  <td>the BACKSPACE key on the terminal</td>
</tr>
<tr>
  <td>$KC_ESC$</td>
  <td>the ESC (escape) key on the terminal</td>
</tr>
</tbody>
</table>
<p>]]
    },

  },

}

data_pt = data_en
