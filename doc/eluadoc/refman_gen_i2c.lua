-- eLua reference manual - I2C module

data_en = 
{

  -- Title
  title = "eLua reference manual - I2C module",

  -- Menu name
  menu_name = "i2c",

  -- Overview
  overview = [[This module contains functions for accessing the I2C interfaces of the eLua CPU.</p>
  <p><span class="warning">IMPORTANT</span>: right now, only master I2C mode is implemented in eLua.]],

  -- Functions
  funcs = 
  {
    { sig = "speed = #i2c.setup#( id, speed )",
      desc = "Setup the I2C interface.",
      args = 
      {
        "$id$ - the ID of the I2C interface.",
        "$speed$ - the clock frequency of the I2C interface. It can be $i2c.FAST$ (400KHz), $i2c.SLOW$ (100KHz) or a number giving the required I2C bus clock speed in Hz.",
      },
      ret = "the actual speed of the I2C interface."
    },

    { sig = "#i2c.start#( id )",
      desc = "Send a START on the specified I2C interface.",
      args = "$id$ - the ID of the I2C interface.",
    },

    { sig = "#i2c.stop#( id )",
      desc = "Send a STOP on the specified I2C interface.",
      args = "$id$ - the ID of the I2C interface.",
    },

    { sig = "acked = #i2c.address#( id, address, direction )",
      desc = "Send an address on the I2C interface.",
      args = 
      {
        "$id$ - the ID of the I2C interface.",
        "$address$ - the address.",
        "$direction$ - $i2c.TRANSMITTER$ if the master wants to send data, or $i2c.RECEIVER$ if the master wants to receive data."
      },
      ret = "$true$ if the address was acknowledged by an I2C slave, $false$ otherwise." 
    },

    { sig = "wrote = #i2c.write#( id, data1, [data2], ..., [datan] )",
      desc = "Writes data to a slave that has already acknowledged an @#i2c.address@i2c.address@ call.",
      args = 
      {
        "$id$ - the ID of the I2C interface.",
        "$data1$ - the data to send. It can be either a number between 0 and 255, a string or a table (array) of numbers.",
        "$data2 (optional)$ - the second data to send.",
        "$datan (optional)$ - the %n%-th data to send."
      },
      ret = "the number of bytes actually written."
    },

    { sig = "data = #i2c.read#( id, numbytes )",
      desc = "Reads a number of bytes from a slave that has already acknowledged an @#i2c.address@i2c.address@ call. It acknowledges all the bytes received except for the last one.", 
      args =
      {
        "$id$ - the ID of the I2C interface.",
        "$numbytes$ - the number of bytes to read."
      },
      ret = "a string with all the data read from the I2C interface."
    }
   
  },

}

data_pt = data_en
