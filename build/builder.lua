-- Generate a C configuration starting from a Lua description file
-- for an eLua board

package.path = package.path .. ";utils/?.lua;build/?.lua"

local comp = require "component"
local ct = require "constants"

-- Initialization
comp.init()

---------------
-- Test test

--[[
Template:

return {
  cpu = 'lm3s8962',
  external_ram = { ... },
  components = {
    shell = { },
    romfs,
    wofs,
    tcpip = {},
    dns,
    dhcp,
  },
  extra = {
    uart_buffers = { ... },
    virtual_uarts = { ... },
    virtual_timers = { ... }
    cpu_constants = { ... }
  },
  modules = {}
}
--]]

local test = {
  cpu = 'lm3s8962',
  components = {
    wofs = true,
    romfs = true,
    shell = { uart = 0, speed = 115200 },
    xmodem = true 
  }
}

print( comp.gen_config( test ) )
