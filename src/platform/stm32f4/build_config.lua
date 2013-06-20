-- This is the platform specific board configuration file
-- It is used by the generic board configuration system (config/)

module( ..., package.seeall )
local at = require "attributes"
local comps = require "components"

-- Add specific components to the 'components' table
function add_platform_components( t, board, cpu )
  t.cdc = comps.cdc_uart()
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
end

-- Return an array of all the available platform modules for the given cpu
function get_platform_modules( board, cpu )
end

