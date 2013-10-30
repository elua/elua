-- eLua platform interface - SPI
-- Make a full description for each language

data_en = 
{
  -- Menu name
  menu_name = "SPI",

  -- Title
  title = "eLua platform interface - SPI",

  -- Overview
  overview = "This part of the platform interface groups functions related to the SPI interface(s) of the MCU.",

  -- Data structures, constants and types
  structures = 
  {
    { text = [[// SPI mode
#define PLATFORM_SPI_MASTER                   1
#define PLATFORM_SPI_SLAVE                    0 ]],
      name = "Chip select",
      desc = "Constants used to select/deselect the SPI SS pin (if applicable)."
    },

    { text = [[// SS values
#define PLATFORM_SPI_SELECT_ON                1
#define PLATFORM_SPI_SELECT_OFF               0]],
      name = "SPI mode",
      desc = "Constants used to select/deselect the SPI SS pin (if applicable)."
    }, 

    { text = "typedef u32 spi_data_type;",
      name = "SPI data type",
      desc = "This is the type of a SPI data word, thus limiting the maximum size of a SPI data work to 32 bits (which should be enough for all practical purposes)."
    }
  },

  -- Functions
  funcs = 
  {
    { sig = "int #platform_spi_exists#( unsigned id );",
      desc = [[Checks if the platform has the hardware SPI specified as argument. Implemented in %src/common.c%, it uses the $NUM_SPI$ macro that must be defined in the
  platform's $cpu_xxx.h$ file (see @arch_overview.html#platforms@here@ for details). For example:</p>
  ~#define NUM_SPI   1      $// The platform has 1 SPI interface$~<p> ]],
      args = "$id$ - SPI interface ID",
      ret = "1 if the SPI interface exists, 0 otherwise"
    },

    { sig = "u32 #platform_spi_setup#( unsigned id, int mode, u32 clock, unsigned cpol, unsigned cpha, unsigned databits );",
      desc = [[This function is used to initialize the parameters of the SPI interface. <span class="warning">NOTE</span>: currently, only master SPI mode is implemented in eLua.]],
      args = 
      {
        "$id$ - SPI interface ID",
        "$mode$ - SPI port mode ($PLATFORM_SPI_MASTER$ or $PLATFORM_SPI_SLAVE$, see @#spi_mode@here@.",
        "$clock$ - clock speed for the SPI interface in master mode.",
        "$cpol$ - SPI clock polarity",
        "$cpha$ - SPI clock phase",
        "$databits$ - length of the SPI data word in bits (usually 8, but configurable on some platforms)."
      },
      ret = "the actual clock set for the SPI interface. Depending on the hardware, this may have a different value than the $clock$ argument."
    },

    {  sig = "spi_data_type #platform_spi_send_recv#( unsigned id, spi_data_type data );",
       desc = "Executes a SPI read/write cycle",
       args = 
       {
         "$id$ - SPI interface ID",
         "$data$ - data to be sent to the SPI interface",
       },
       ret = "data read from the SPI interface"
    },

    { sig = "void #platform_spi_select#( unsigned id, int is_select );",
      desc = [[For platforms that have a dedicates SS (Slave Select) pin in master SPI mode that can be controlled manually, this function should enable/disable this pin. If this functionality
  does not exist in hardware this function does nothing.]],
      args =
      {
        "$id$ - SPI interface ID.",
        "$is_select$ - $PLATFORM_SPI_SELECT_ON$ to select, $PLATFORM_SPI_SELECT_OFF$ to deselect , see @#chip_select@here@." 
      },
    }
  }
}

data_pt = data_en
