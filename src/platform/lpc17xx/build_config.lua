-- This is the platform specific board configuration file
-- It is used by the generic board configuration system (config/)

module( ..., package.seeall )
local at = require "attributes"
local comps = require "components"

-- Add specific components to the 'components' table
function add_platform_components( t, board, cpu )
  t.lpc17xx_semifs = { macro = "BUILD_SEMIFS" }
  t.cdc = comps.cdc_uart()
end

-- Add specific configuration to the 'configs' table
function add_platform_configs( t, board, cpu )
  t.map_pins = {
    attrs = {
      port = at.array_of( at.int_attr( 'LPC17XX_MAP_PORT' )),
      pin  = at.array_of( at.int_attr( 'LPC17XX_MAP_PIN' )),
      pinfunction = at.array_of( at.int_attr( 'LPC17XX_MAP_PINFUNCTION' ))
    }
  }
end

-- Return an array of all the available platform modules for the given cpu
function get_platform_modules( board, cpu )
  local m = { pio = { map = "lpc17xx_pio_map", open = "luaopen_lpc17xx_pio" } }
  board = board:upper()
  if board == 'MBED' then
    m.pio = { map = "mbed_pio_map", open = "luaopen_mbed_pio" }
  end
  return m
end

