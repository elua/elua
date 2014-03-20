-- This is the platform specific board configuration file
-- It is used by the generic board configuration system (config/)

module( ..., package.seeall )
local at = require "attributes"
local comps = require "components"

-- Add specific components to the 'components' table
function add_platform_components( t, board, cpu )
  t.cdc = comps.cdc_uart()
  t.stm32f4_enc = { macro = 'ENABLE_ENC' }
end

-- Add specific configuration to the 'configs' table
function add_platform_configs( t, board, cpu )
  t.stm32f4_uart_pins = {
    attrs = {
      con_rx_port = at.int_attr( 'STM32F4_CON_RX_PORT', 0 ),
      con_rx_pin = at.int_attr( 'STM32F4_CON_RX_PIN', 0 ),
      con_tx_port = at.int_attr( 'STM32F4_CON_TX_PORT', 0 ),
      con_tx_pin = at.int_attr( 'STM32F4_CON_TX_PIN', 0 )
    },
    required = { con_rx_port = 1, con_rx_pin = 7, con_tx_port = 1, con_tx_pin = 6 }
  }
  t.clocks.attrs.internal = at.make_optional( at.int_attr( 'ELUA_BOARD_INTERNAL_CLOCK_HZ', 1 ) )
end

-- Return an array of all the available platform modules for the given cpu
function get_platform_modules( board, cpu )
  return { pio = { lib = '"pio"', map = "stm32_pio_map", open = false },
           cpu = { lib = '"cpu"', map = "stm32_cpu_map", open = "luaopen_stm32_cpu" },
           enc = { guards = { 'ENABLE_ENC' }, lib = '"enc"' } }
end

