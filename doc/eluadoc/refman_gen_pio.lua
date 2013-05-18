-- eLua reference manual - pio module

data_en = 
{

  -- Title
  title = "eLua reference manual - pio module",

  -- Menu name
  menu_name = "pio",

  -- Overview
  overview = [[This module contains functions for accessing the CPU's PIO (Programmable Input Output) pins. It contains two set of functions with identical
  names and behaviour. One set groups the functions used to access individual pins from ports, the other groups the functions used to access full ports.</p>
  <p>With the $pio$ module, you specifiy names of $ports$ as they appear in your eLua's CPU datasheet. For example, if your CPU's ports are named $PA, PB$ 
  and $PC$, you can reffer to them using $pio.PA$, $pio.PB$ and $pio.PC$, respectively. If your CPU uses $P0$, $P1$, $P2$ instead of $PA$, $PB$ and $PC$, 
  you can simply use $pio.P0$, $pio.P1$ and $pio.P2$ instead.</p>
  <p>You can also reffer to individual $pins$ instead of ports. With the same notation as above, $pio.PA_0$ refers to the first pin of port $PA$, 
  $P0_15$ refers to the 16th pin of port $P0$ and so on.
  ]],

  -- Functions
  funcs = 
  {
    { sig = "#pio.pin.setdir#( direction, pin1, pin2, ..., pinn )",
      desc = "Set pin(s) direction",
      args =
      {
        "$direction$ - the pin direction, can be either $pio.INPUT$ or $pio.OUTPUT$",
        "$pin1$ - the first pin",
        "$pin2 (optional)$ - the second pin",
        "$pinn (optional)$ - the %n%-th pin"
      }
    },

    { sig = "#pio.pin.output#( pin1, pin2, ..., pinn )",
      desc = "Set pin(s) direction to output",
      args =
      {
        "$pin1$ - the first pin",
        "$pin2 (optional)$ - the second pin",
        "$pinn (optional)$ - the %n%-th pin",
      }
    },

    { sig = "#pio.pin.input#( pin1, pin2, ..., pinn )",
      desc = "Set pin(s) direction to input",
      args =
      {
        "$pin1$ - the first pin",
        "$pin2 (optional)$ - the second pin",
        "$pinn (optional)$ - the %n%-th pin",
      }
    },

    { sig = "#pio.pin.setpull#( type, pin1, pin2, ..., pinn )",
      desc = "Enable/disable pullups/pulldowns on the specified pin(s)",    
      args = 
      {
        [[$type$ - 'pull' type, can be either $pio.PULLUP$ to enable pullups, $pio.PULLDOWN$ to enable pulldowns, or $pio.NOPULL$ to disable both pullups and
        pulldowns]],
        "$pin1$ - the first pin",
        "$pin2 (optional)$ - the second pin",
        "$pinn (optional)$ - the %n%-th pin",
      }
    },

    { sig = "#pio.pin.setval#( value, pin1, pin2, ..., pinn )",
      desc = "Set pin(s) value",
      args=
      {
        "$value$ - pin value, can be either 0 or 1",
        "$pin1$ - the first pin",
        "$pin2 (optional)$ - the second pin",
        "$pinn (optional)$ - the %n%-th pin"
      }
    },

    { sig = "val1, val2, ..., valn = #pio.pin.getval#( pin1, pin2, ..., pinn )",
      desc = "Get value of pin(s)",
      args =
      {
        "$pin1$ - the first pin",
        "$pin2 (optional)$ - the second pin",
        "$pinn (optional)$ - the %n%-th pin",
      },
      ret = "The value(s) of the pin(s), either 0 or 1"
    },

    { sig = "#pio.pin.sethigh#( pin1, pin2, ..., pinn )",
      desc = "Set pin(s) to 1 (high)",
      args = 
      {
        "$pin1$ - the first pin",
        "$pin2 (optional)$ - the second pin",
        "$pinn (optional)$ - the %n%-th pin"
      }
    },

    { sig = "#pio.pin.setlow#( pin1, pin2, ..., pinn )",
      desc = "Set pin(s) to 0 (low)",
      args = 
      {
        "$pin1$ - the first pin",
        "$pin2 (optional)$ - the second pin",
        "$pinn (optional)$ - the %n%-th pin"
      }
    },

    { sig = "#pio.port.setdir#( direction, port1, port2, ..., portn )",
      desc = "Set port(s) direction",
      args =
      {
        "$direction$ - the port direction, can be either $pio.INPUT$ or $pio.OUTPUT$",
        "$port1$ - the first port",
        "$port2 (optional)$ - the second port",
        "$portn (optional)$ - the %n%-th port"
      }
    },

    { sig = "#pio.port.output#( port1, port2, ..., portn )",
      desc = "Set port(s) direction to output",
      args =
      {
        "$port1$ - the first port",
        "$port2 (optional)$ - the second port",
        "$portn (optional)$ - the %n%-th port"
      }
    },

    { sig = "#pio.port.input#( port1, port2, ..., portn )",
      desc = "Set port(s) direction to input",
      args =
      {
        "$port1$ - the first port",
        "$port2 (optional)$ - the second port",
        "$portn (optional)$ - the %n%-th port"
      }
    },

    { sig = "#pio.port.setpull#( type, port1, port2, ..., portn )",
      desc = "Enable/disable pullups/pulldowns on the specified port(s)",    
      args = 
      {
        [[$type$ - 'pull' type, can be either $pio.PULLUP$ to enable pullups, $pio.PULLDOWN$ to enable pulldowns, or $pio.NOPULL$ to disable both pullups and
        pulldowns]],
        "$port1$ - the first port",
        "$port2 (optional)$ - the second port",
        "$portn (optional)$ - the %n%-th port",
      }
    },

    { sig = "#pio.port.setval#( value, port1, port2, ..., portn )",
      desc = "Set port(s) value",
      args=
      {
        "$value$ - port value",
        "$port1$ - the first port",
        "$port2 (optional)$ - the second port",
        "$portn (optional)$ - the %n%-th port"
      }
    },

    { sig = "val1, val2, ..., valn = #pio.port.getval#( port1, port2, ..., portn )",
      desc = "Get value of port(s)",
      args =
      {
        "$port1$ - the first port",
        "$port2 (optional)$ - the second port",
        "$portn (optional)$ - the %n%-th port",
      },
      ret = "The value(s) of the port(s)"
    },

    { sig = "#pio.port.sethigh#( port1, port2, ..., portn )",
      desc = "Set port(s) to all 1 (high)",
      args = 
      {
        "$port1$ - the first port",
        "$port2 (optional)$ - the second port",
        "$portn (optional)$ - the %n%-th port"
      }
    },

    { sig = "#pio.port.setlow#( port1, port2, ..., portn )",
      desc = "Set port(s) to all 0 (low)",
      args = 
      {
        "$port1$ - the first port",
        "$port2 (optional)$ - the second port",
        "$portn (optional)$ - the %n%-th port"
      }
    },

    { sig = "port, pin = #pio.decode#( resnum )",
      desc = "Convert a PIO resource number to the corresponding port and pin. This is most commonly used in GPIO edge interrupt routines to convert the Lua interrupt routine's argument to the port and pin that caused the interrupt but it can also be used on the values returned by the pin names pio.PA_0, pio.P2_15 and so on.",
      args =
      {
        "$resnum$ - the resource number of the pin",
      },
      ret =
      {
        "$port$ - the index of the port, starting from 0 (so port A is 0, port B is 1 and so on)",
        "$pin$ - the pin number, usually from 0 to 31",
      },
    },
   
  }

}

data_pt = data_en
