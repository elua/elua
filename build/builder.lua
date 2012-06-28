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

--[[ platform_conf.h template
#include various files (buf, type...)

#include ELUA_CPU_HEADER
#include ELUA_BOARD_HEADER
#include "platform_post_config.h"
#include "platform_generic.h"
#include "platform_interrupts.h"
--]]

local test = {
  cpu = 'stm32f103',
  config = {
    extram = { start = 0, size = 256 * 1024 },
    vtmr = { num = 4, freq = 10 },
    egc = { mode = "full", limit = 40 * 1024 }
  },
  components = {
    wofs = true,
    romfs = true,
    shell = { uart = 0, speed = 115200 },
    xmodem = true,
    term = { lines = 25, cols = 80 },
    cints = true,
    luaints = { queue_size = 32 },
    linenoise = { shell_lines = 10, lua_lines = 50 },
    rfs = { uart = 1, speed = 115200, buf_size = 512 }
  }
}

print( comp.gen_config( test ) )

