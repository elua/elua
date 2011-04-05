-- Serial console component generator

module( ..., package.seeall )
local sf = string.format
local compgen = require "gen-comp"
local utils = require "utils"
local opt = require "option"

local cgen = {}
local base = compgen.compgen
setmetatable( cgen, { __index = base } )

new = function( ptable )
  local self = {}
  setmetatable( self, { __index = cgen } )
  base.init_instance( self, 'console', ptable )
  self:set_friendly_name( 'Serial console' )
  self:set_enable_macro( 'BUILD_CON_GENERIC' )
  self.timer_id_text = 'Timer used for UART operations'
  return self
end

-- Notification function: look for VTMR
cgen.notification = function( self, component, state )
  if component == self.COMP_VTMR then
    self:check_timer_change( 'CON_TIMER_ID', self.timer_id_text, state )
  end
end

-- Extra dependency checking
cgen.check_explicit_deps = function( self )
  local cgen = assert( self.find_generator( self.COMP_TELNET ) )
  if cgen:is_enabled() then
    return false, "Telnet support and serial console can't be enabled at the same time"
  end
  return true
end

-- Initialization function
cgen.init = function( self, t )
  base.init( self, t )
  self:set_help( 'Serial system console (for the (e)Lua shell, ANSI terminal emulation and other services).' )
  self:add_deps{ self.IO_TIMER, self.IO_UART }
  local uart_gen = assert( self.find_generator( self.IO_UART ) )
  self:add_option( opt.int_option( 'CON_UART_ID', 0, 'UART ID of the serial console', { min = 0,  max = uart_gen:get_num() - 1 } ) )
  self:add_option( opt.int_option( 'CON_UART_SPEED', 115200, 'UART speed', { min = 1200 } ) )
  self:add_option( self.option_timers( 'CON_TIMER_ID', self.timer_id_text ) )
  self:add_option( self.option_flow_control( 'CON_FLOW_TYPE', 'Flow control on the UART console' ) )
end

cgen.__type = function()
  return "gen-comp-console"
end

