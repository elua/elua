
--[[

Notes:

This is the platform specific board configuration file It is used by
the generic board configuration system (config/)

Taken from lm3s/build_config.lua. Adapted for the XMC4000 platform by
Raman

--]]

module( ..., package.seeall )
local comps = require "components"
local at = require "attributes"

-- Add specific components to the 'components' table
function add_platform_components( t, board, cpu )
  board = board:upper()
  if board == 'XMC4500-HEXAGON' then
    t.xmc45_pot = { macro = 'ENABLE_POTENTIOMETER' }
    t.xmc45_dts = { macro = 'ENABLE_DTS' }
    t.xmc45_rtc = { macro = 'ENABLE_RTC' }
  end
end

-- Add specific configuration to the 'configs' table
function add_platform_configs( t, board, cpu )
  -- Blank now; Let's get back to it later.
end

-- Return an array of all the available platform modules for the given cpu
function get_platform_modules( board, cpu )
  m = { }
  board = board:upper()
  if board == 'XMC4500-HEXAGON' then
    m.pot = { guards = { 'ENABLE_POTENTIOMETER' }, lib = '"pot"', open = false }
    m.dts = { guards = { 'ENABLE_DTS' }, lib = '"dts"', open = false }
    m.rtc = { guards = { 'ENABLE_RTC' }, lib = '"rtc"', open = false }
  end
  return m
end
