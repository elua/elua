-- eLua platform interface - CAN
-- Make a full description for each language

data_en = 
{
  -- Menu name
  menu_name = "CAN",

  -- Title
  title = "eLua platform interface - CAN",

  -- Overview
  overview = "This part of the platform interface groups functions related to the CAN interface(s) of the MCU..",

  -- Data structures, constants and types
  structures = 
  {
    { text = [[// eLua CAN ID types
enum
{
  ELUA_CAN_ID_STD = 0,
  ELUA_CAN_ID_EXT
};
]],
      name = "CAN ID types",
      desc = "Constants used to define whether the message ID is standard or extended.."
    }
  },

  -- Functions
  funcs = 
  {
    { sig = "int #platform_can_exists#( unsigned id );",
      desc = [[Checks if the platform has the hardware CAN specified as argument. Implemented in %src/common.c%, it uses the $NUM_CAN$ macro that must be defined in the
  platform's $cpu_xxx.h$ file (see @arch_overview.html#platforms@here@ for details). For example:</p>
  ~#define NUM_CAN   1      $// The platform has one CAN interface$~<p> ]],
      args = "$id$ - CAN interface ID.",
      ret = "1 if the CAN interface exists, 0 otherwise"
    },

    { sig = "u32 #platform_can_setup#( unsigned id, u32 clock );",
      desc = [[This function is used to initialize the CAN hardware and set the bus clock.]],
      args = 
      {
        "$id$ - CAN interface ID.",
        "$clock$ - the clock of the CAN bus, maximum speed is generally 1000000 (1 Mbit)"
      },
      ret = "the actual speed set for the CAN interface. Depending on the hardware, this may have a different value than the $clock$ argument."
    },

    {  sig = "int #platform_can_send#( unsigned id, u32 canid, u8 idtype, u8 len, const u8 *data );",
       desc = "Send message over the CAN bus.",
       args =
       {
          "$id$ - CAN interface ID.",
          "$canid$ - CAN identifier number.",
          "$canidtype$ - identifier type as defined @#can_id_types@here@",
          "$len$ - message length in bytes (8 or fewer)",
          "$message$ - pointer to message, 8 or fewer bytes in length"
       },
       ret = "PLATFORM_OK for success, PLATFORM_ERR if the message wasn't sent."
    },

     {  sig = "int #platform_can_recv#( unsigned id, u32 *canid, u8 *idtype, u8 *len, u8 *data );",
        desc = "Receive CAN bus message.",
        args =
       {
          "$id$ - CAN interface ID.",
          "$canid$ - pointer where CAN identifier number will be written.",
          "$canidtype$ - pointer where identifier type as defined @#can_id_types@here@ will be written",
          "$len$ - pointer where message length in bytes will be written",
          "$message$ - pointer to message buffer (8 bytes in lenth)"
       },
       ret = "PLATFORM_OK for success, PLATFORM_UNDERFLOW for error. (see @arch_platform_ll.html@here@ for details)"
    },
  }
}

