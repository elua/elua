-- eLua reference manual - SPI module

data_en = 
{

  -- Title
  title = "eLua reference manual - SPI module",

  -- Menu name
  menu_name = "spi",

  -- Overview
  overview = [[This module contains functions for accessing the SPI interfaces of the eLua CPU.</p>
  <p><span class="warning">IMPORTANT</span>: right now, only master SPI mode is implemented in eLua.]],

  -- Functions
  funcs = 
  {
    { sig = "clock = #spi.setup#( id, type, clock, cpol, cpha, databits )",
      desc = "Setup the SPI interface",
      args = 
      {
        "$id$ - the ID of the SPI interface.",
        "$type$ - SPI interface type, can be either $spi.MASTER$ or $spi.SLAVE$. $NOTE: currently, only master SPI mode is supported$.",
        "$clock$ - the clock of the SPI interface.",
        "$cpol$ - the clock polarity (0 or 1).",
        "$cpha$ - the clock phase (0 or 1).",
        "$databits$ - the length of the SPI data word.",
      },
      ret = "The actual clock set on the SPI interface. Depending on the hardware, this might have a different value than the $clock$ parameter."
    },

    { sig = "#spi.sson#( id )",
      desc = "Select the SS line (Slave Select) of the SPI interface. This is only applicable for SPI interfaces with a dedicated SS pin.",
      args = "$id$ - the ID of the SPI interface.",
    },

    { sig = "#spi.ssoff#( id )",
      desc = "Deselect the SS line (Slave Select) of the SPI interface. This is only applicable for SPI interfaces with a dedicated SS pin.",
      args = "$id$ - the ID of the SPI interface.",
    },

    { sig = "#spi.write#( id, data1, [data2], ..., [datan] )",
      desc = "Write one or more strings/numbers to the SPI interface.",
      args = 
      {
        "$id$ - the ID of the SPI interface.",
        "$data1$ - the first string/number to send.",
        "$data2 (optional)$ - the second string/number to send.",
        "$datan (optional)$ - the %n%-th string/number to send."
      },
    },

    { sig = "#spi.readwrite#( id, data1, [data2], ..., [datan] )",
      desc = "Write one or more strings/numbers to the SPI interface and return the data read from the same interface.",
      args =
      {
        "$id$ - the ID of the SPI interface.",
        "$data1$ - the first string/number to send.",
        "$data2 (optional)$ - the second string/number to send.",
        "$datan (optional)$ - the %n%-th string/number to send."
      },
      ret = "An array with all the data read from the SPI interface."
    }
   
  },

}

data_pt = data_en
