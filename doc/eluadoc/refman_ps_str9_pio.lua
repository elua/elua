-- eLua reference manual - platform data

data_en = 
{

  -- Title
  title = "eLua reference manual - STR9 pio module",

  -- Menu name
  menu_name = "pio",

  -- Overview
  overview = [[This module contains functions for accessing the particular features of the PIO subsystem of the STR9 family of CPUs. This subsystem is very flexible, allowing things like
  configurable output types (push-pull or open collector), multiple alternate functions for the PIO pins, and others. For a full description of the STR9 PIO module check the STR9 CPU
  Reference manual, available from ST at @http://www.st.com/mcu/devicedocs-STR912FAW44-101.html@this address@. Note that this module is a supplement of the platform independent 
  @refman_gen_pio.html@pio@ module, not a replacement. Use this module only for setting up the STR9 PIO pins, and the @refman_gen_pio.html@pio@ module for all the other PIO related operations.]],

  -- Functions
  funcs = 
  {
    { sig = "#str9.pio.setpin#( pin, direction, type, ipconnected, alternate )",
      desc = "Setup the pin.",
      args = 
      {
        "$pin$ - the pin, as encoded in the @refman_gen_pio.html@pio@ module.",
        "$direction$ - the pin direction, can be either $str9.pio.INPUT$ or $str9.pio.OUTPUT$,",
        "$type$ - the type of the pin, can be either $str9.pio.OUTPUT_PUSHPULL$ (push-pull) or $str9.pio.OUTPUT_OC$ (open collector).",
        "$ipconnected$ - $true$ to connect the pin to its corresponding peripheral(s), $false$ otherwise.",
        "$alternate$ - alternate pin function. Must be either $str9.pio.ALT_INPUT$, $str9.pio.ALT_OUTPUT1$, $str9.pio.ALT_OUTPUT2$, $str9.pio.ALT_OUTPUT3$."
      }
    },

  },
}

data_pt = data_en
