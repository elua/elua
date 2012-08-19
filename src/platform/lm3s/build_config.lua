-- This is the platform specific board configuration file
-- It is used by the generic board configuration system (config/)

module( ..., package.seeall )
local comps = require "components"

-- Add specific components to the 'components' table
function add_platform_components( t, board, cpu )
  t.cdc = comps.cdc_uart()
  board = board:upper()
  if board == 'EK-LM3S1968' or board == 'EK-LM3S6965' or board == 'EK-LM3S8962' then
    t.lm3s_disp = { macro = 'ENABLE_DISP' }
  end
end

-- Add specific configuration to the 'configs' table
function add_platform_configs( t, board, cpu )
end

-- Return an array of all the available platform modules for the given cpu
function get_platform_modules( board, cpu )
  board = board:upper()
  if board == 'EK-LM3S1968' or board == 'EK-LM3S6965' or board == 'EK-LM3S8962' then
    return {
      disp = { guards = { 'ENABLE_DISP' }, lib = '"disp"', open = false }
    }
  end
end

