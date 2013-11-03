-- eLua platform interface - I2C
-- Make a full description for each language

data_en = 
{
  -- Menu name
  menu_name = "I2C",

  -- Title
  title = "eLua platform interface - I2C",

  -- Overview
  overview = "This part of the platform interface groups functions related to the I2C interface(s) of the MCU. Currently only the I2C master mode is supported by eLua.",

  -- Data structures, constants and types
  structures = 
  {
    { text = [[// I2C speed
enum
{
  PLATFORM_I2C_SPEED_SLOW = 100000,
  PLATFORM_I2C_SPEED_FAST = 400000
};]],
      name = "I2C speed",
      desc = "Constants used to configure the speed of the I2C interface."
    },

    { text = [[// I2C direction
enum
{
  PLATFORM_I2C_DIRECTION_TRANSMITTER,
  PLATFORM_I2C_DIRECTION_RECEIVER
};
]],
      name = "I2C transfer direction",
      desc = "Constants used to select the I2C master transfer direction (transmitter or receiver)."
    }, 

  },

  -- Functions
  funcs = 
  {
    { sig = "int #platform_i2c_exists#( unsigned id );",
      desc = [[Checks if the platform has the hardware I2C specified as argument. Implemented in %src/common.c%, it uses the $NUM_I2C$ macro that must be defined in the
  platform's $cpu_xxx.h$ file (see @arch_overview.html#platforms@here@ for details). For example:</p>
  ~#define NUM_I2C   1      $// The platform has one I2C interface$~<p> ]],
      args = "$id$ - I2C interface ID.",
      ret = "1 if the I2C interface exists, 0 otherwise"
    },

    { sig = "u32 #platform_i2c_setup#( unsigned id, u23 speed );",
      desc = [[This function is used to initialize the parameters of the I2C interface. <span class="warning">NOTE</span>: currently, only master I2C mode is implemented in eLua.]],
      args = 
      {
        "$id$ - I2C interface ID.",
        "$speed$ - the speed of the interface, can be either $PLATFORM_I2C_SPEED_SLOW$ or $PLATFORM_I2C_SPEED_FAST$ as defined @#i2c_speed@here@"
      },
      ret = "the actual speed set for the I2C interface. Depending on the hardware, this may have a different value than the $speed$ argument."
    },

    {  sig = "void #platform_i2c_send_start#( unsigned id );",
       desc = "Send an I2C START condition on the specified interface.",
       args = "$id$ - I2C interface ID."
    },

     { sig = "void #platform_i2c_send_stop#( unsigned id );",
       desc = "Send an I2C STOP condition on the specified interface.",
       args = "$id$ - I2C interface ID."
    },
   
    { sig = "int #platform_i2c_send_address#( unsigned id, u16 address, int direction );",
      desc = [[Send an address on the I2C with the specified transfer direction (transmitter or receiver).]],
      args =
      {
        "$id$ - I2C interface ID.",
        "$address$ - I2C peripheral address.",
        "$direction$ - transfer direction, either $PLATFORM_I2C_DIRECTION_TRANSMITTER$ or $PLATFORM_I2C_DIRECTION_RECEIVER$ as defined @#i2c_transfer_direction@here@."
      },
      ret = "1 for success, 0 for error."
    },

    { sig = "int #platform_i2c_send_byte#( unsigned id, u8 data );",
      desc = "Send a byte on the I2C interface.",
      args = 
      {
        "$id$ - I2C interface ID.",
        "$data$ - the byte to send.",
      },
      ret = "1 for success, 0 for error.",
    },

    { sig = "int #platform_i2c_recv_byte#( unsigned id, int ack );",
      desc = "Receive a byte from the I2C interface and send a positive (ACK) or negative (NAK) acknowledgement.",
      args =
      {
        "$id$ - I2C interface ID.",
        "$ack$ - 1 to send ACK, 0 to send NAK. If $ACK$ is 0 a STOP condition will automatically be generated after the NAK."
      },
      ret = "1 for success, 0 for error."
    }
  }
}

