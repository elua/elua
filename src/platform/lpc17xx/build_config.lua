-- This is the platform specific board configuration file
-- It is used by the generic board configuration system (config/)

module( ..., package.seeall )

-- Add specific components to the 'components' table
function add_platform_components( t, board, cpu )
  t.lpc17xx_semifs = { macro = "BUILD_SEMIFS" }
end

-- Add specific configuration to the 'configs' table
function add_platform_configs( t, board, cpu )

end

-- Return an array of all the available platform modules for the given cpu
function get_platform_modules( board, cpu )
  local m = { }
  board = board:upper()
  if board == 'MBED' then
    m.pio = { map = "mbed_pio_map", open = "luaopen_mbed_pio" }
  end
  return m
end

