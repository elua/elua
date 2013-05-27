-- This is the platform specific board configuration file
-- It is used by the generic board configuration system (config/)

module( ..., package.seeall )
local comps = require "components"

-- Add specific components to the 'components' table
function add_platform_components( t )
  t.cdc = comps.cdc_uart()
  t.avr32_lcd = { macro = "BUILD_LCD" }
  t.avr32_rtc = { macro = "BUILD_RTC" }
end

-- Add specific configuration to the 'configs' table
function add_platform_configs( t )
end

-- Return an array of all the available platform modules for the given cpu
function get_platform_modules( cpu )
  return {
    lcd = { guards = { "BUILD_LCD" }, lib = '"lcd"', open = false },
    rtc = { guards = { "BUILD_RTC" }, lib = '"rtc"', open = false }
  }
end

