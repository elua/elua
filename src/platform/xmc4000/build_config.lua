
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
  if board == 'XMC4500-HEXAGON' or board == 'XMC4500-HEXAGON-SDRAM' or board == 'XMC4500-PROTOCARD-V2' then
    t.xmc45_dts = { macro = 'ENABLE_DTS' }
    t.xmc45_rtc = { macro = 'ENABLE_RTC' }
  end
  if board == 'XMC4500-HEXAGON' then
    t.xmc45_pot = { macro = 'ENABLE_POTENTIOMETER' }
    t.xmc45_disp = { macro = 'ENABLE_OLED_DISPLAY' }
  end
  if board == 'XMC4700-RELAX' then
    t.xmc47_dac = { macro = 'ENABLE_DAC' }
  end
  if board == 'XMC4300-RELAX-ETHERCAT' then
    t.xmc43_dac = { macro = 'ENABLE_DAC' }
    t.xmc43_lcd = { macro = 'ENABLE_LCD' }
    t.xmc43_capture = { macro = 'ENABLE_CAPTURE' }
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
  if board == 'XMC4500-HEXAGON' or board == 'XMC4500-HEXAGON-SDRAM' or board == 'XMC4500-PROTOCARD-V2' then
    m.dts = { guards = { 'ENABLE_DTS' }, lib = '"dts"', open = false }
    m.rtc = { guards = { 'ENABLE_RTC' }, lib = '"rtc"', open = false }
  end
  if board == 'XMC4500-HEXAGON' then
    m.pot = { guards = { 'ENABLE_POTENTIOMETER' }, lib = '"pot"', open = false }
    m.disp = { guards = { 'ENABLE_OLED_DISPLAY' }, lib = '"disp"', open = false }
  end
  if board == 'XMC4700-RELAX' or board == 'XMC4300-RELAX-ETHERCAT' then
    m.dac = { guards = { 'ENABLE_DAC' }, lib = '"dac"', open = false }
    m.lcd = { guards = { 'ENABLE_LCD' }, lib = '"lcd"', open = false }
    m.capture = { guards = { 'ENABLE_CAPTURE' }, lib = '"capture"', open = false }
  end
  return m
end
