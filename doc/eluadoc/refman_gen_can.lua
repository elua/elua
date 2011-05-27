-- eLua reference manual - CAN module

data_en = 
{

  -- Title
  title = "eLua reference manual - CAN module",

  -- Menu name
  menu_name = "can",

  -- Overview
  overview = [[This module contains functions for accessing the CAN interfaces of the eLua CPU]],
  
  -- Structures
  structures =
  {
    { text = [[// eLua CAN ID types
enum
{
  ELUA_CAN_ID_STD = 0,      // exported as $can.ID_STD$
  ELUA_CAN_ID_EXT,          // exported as $can.ID_EXT$
};]],
      name = "CAN ID types",
      desc = "These are the CAN identifier types supported by eLua. Standard identifiers are 11 bits in length, extended identifiers are 29 bits.",
    }
  },

  -- Functions
  funcs = 
  {
    { sig = "clock = #can.setup#( id, clock )",
      desc = "Setup the CAN interface",
      args = 
      {
        "$id$ - the ID of the CAN interface.",
        "$clock$ - the clock of the CAN interface.",
      },
      ret = "The actual clock set on the CAN interface. Depending on the hardware, this might have a different value than the $clock$ parameter."
    },

    { sig = "#can.send#( id, canid, canidtype, message )",
      desc = "Send message over the CAN bus.",
      args = 
      {
        "$id$ - the ID of the CAN interface.",
        "$canid$ - CAN identifier number.",
        "$canidtype$ - identifier type as defined @#can_id_types@here@.",
        "$message$ - message in string format, 8 or fewer bytes."
      },
    },

    { sig = "canid, canidtype, message = #can.recv#( id )",
      desc = "Receive CAN bus message.",
      args =
      {
        "$id$ - the ID of the CAN interface.",
      },
      ret =
      {
        "$canid$ - CAN identifier number.",
        "$canidtype$ - identifier type as defined @#can_id_types@here@.",
        "$message$ - message in string format, 8 or fewer bytes."
      }
    }
   
  },
}

data_pt = data_en
