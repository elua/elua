-- eLua reference manual - bit module

data_en = 
{

  -- Title
  title = "eLua reference manual - bit module",

  -- Menu name
  menu_name = "bit",

  -- Overview
  overview = [[Since Lua doesn't have (yet) built-in capabilities for bit operations, the $bit$ module was added to eLua to fill this gap. It is based on the ^http://luaforge.net/projects/bitlib^bitlib^
  library written by Reuben Thomas (slightly adapted to eLua) and provides basic bit operations (like setting and clearing bits) and bitwise operations.]],

  -- Functions
  funcs = 
  {
    { sig = "number = #bit.bit#( position )",
      desc = "Generate a number with a 1 bit (used for mask generation). Equivalent to %1 <<<< position% in C.",
      args = "$position$ - position of the bit that will be set to 1.",
      ret = "$number$ - a number with only one 1 bit at $position$ (the rest are set to 0)."
    },

    { sig = "flag = #bit.isset#( value, position )",
      desc = "Test if a given bit is set.",
      args = 
      {
        "$value$ - the value to test.",
        "$position$ - bit position to test."
      },
      ret = "$boolean$ - true if the bit at the given position is 1, false otherwise."
    },

    { sig = "flag = #bit.isclear#( value, position )",
      desc = "Test if a given bit is cleared.",
      args = 
      {
        "$value$ - the value to test.",
        "$position$ - bit position to test."
      },
      ret = "$boolean$ - true if the bit at the given position is 0, false othewise."
    },

    { sig = "number = #bit.set#( value, pos1, pos2, ..., posn )",
      desc = "Set bits in a number.",
      args =
      {
        "$value$ - the base number.",
        "$pos1$ - position of the first bit to set.",
        "$pos2$ - position of the second bit to set.",
        "$posn$ - position of the nth bit to set."
      },
      ret = "$number$ - the number with the bit(s) set in the given position(s)."
    },

    { sig = "number = #bit.clear#( value, pos1, pos2, ..., posn )",
      desc = "Clear bits in a number.",
      args = 
      {
        "$value$ - the base number.",
        "$pos1$ - position of the first bit to clear.",
        "$pos2$ - position of the second bit to clear.",
        "$posn$ - position of thet nth bit to clear.",
      },
      ret = "$number$ - the number with the bit(s) cleared in the given position(s)."
    },

    { sig = "number = #bit.bnot#( value )",
      desc = "Bitwise negation, equivalent to %~~value% in C.",
      args = "$value$ - the number to negate.",
      ret = "$number$ - the bitwise negated value of the number.",
    },

    { sig = "number = #bit.band#( val1, val2, ... valn )",
      desc = "Bitwise AND, equivalent to %val1 & val2 & ... & valn% in C.",
      args = 
      {
        "$val1$ - first AND argument.",
        "$val2$ - second AND argument.",
        "$valn$ - nth AND argument.",
      },
      ret = "$number$ - the bitwise AND of all the arguments."
    },

    { sig = "number = #bit.bor#( val1, val2, ... valn )",
      desc = "Bitwise OR, equivalent to %val1 | val2 | ... | valn% in C.",
      args = 
      {
        "$val1$ - first OR argument.",
        "$val2$ - second OR argument.",
        "$valn$ - nth OR argument."
      },
      ret = "$number$ - the bitwise OR of all the arguments."
    },

    { sig = "number = #bit.bxor#( val1, val2, ... valn )",
      desc = "Bitwise exclusive OR (XOR), equivalent to %val1 ^^ val2 ^^ ... ^^ valn% in C.",
      args = 
      {
        "$val1$ - first XOR argument.",
        "$val2$ - second XOR argument.",
        "$valn$ - nth XOR argument."
      },
      ret = "$number$ - the bitwise exclusive OR of all the arguments."
    },

    { sig = "number = #bit.lshift#( value, shift )",
      desc = "Left-shift a number, equivalent to %value << shift% in C.",
      args = 
      {
        "$value$ - the value to shift.",
        "$shift$ - positions to shift.",
      },
      ret = "$number$ - the number shifted left",
    },

    { sig = "number = #bit.rshift#( value, shift )",
      desc = "Logical right shift a number, equivalent to %( unsigned )value >>>> shift% in C.",
      args = 
      {
        "$value$ - the value to shift.",
        "$shift$ - positions to shift.",
      },
      ret = "$number$ - the number shifted right (logically)."
    },

    { sig = "number = #bit.arshift#( value, shift )",
      desc = "Arithmetic right shift a number equivalent to %value >>>> shift% in C.",
      args = 
      {
        "$value$ - the value to shift.",
        "$shift$ - positions to shift."
      },
      ret = "$number$ - the number shifted right (arithmetically)."
    }
  }
}

data_pt = data_en
