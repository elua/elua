-- XMODEM component generator

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
  base.init_instance( self, 'xmodem', ptable )
  self:set_enable_macro( 'BUILD_XMODEM' )
  self:set_friendly_name( 'XMODEM support' )
  self.timer_id_text = 'Timer used for XMODEM operations'
  return self
end

-- Notification function: look for VTMR
cgen.notification = function( self, component, state )
  if component == self.COMP_VTMR then
    self:check_timer_change( 'XMODEM_TIMER_ID', self.timer_id_text, state )
  end
end

-- Initialization function
cgen.init = function( self, t )
  base.init( self, t )
  self:add_deps{ self.IO_UART, self.IO_TIMER }
  self:set_help( 'XMODEM (receive and execute Lua source file via the XMODEM protocol).' )
  local uart_gen = assert( self.find_generator( self.IO_UART ) )
  self:add_option( opt.int_option( 'XMODEM_UART_ID', 0, 'XMODEM UART port', { min = 0,  max = uart_gen:get_num() - 1 } ) )
  self:add_option( opt.int_option( 'XMODEM_UART_SPEED', 115200, 'XMODEM UART speed', { min = 1200 } ) )
  self:add_option( self.option_timers( 'XMODEM_TIMER_ID', self.timer_id_text ) )
end

cgen.__type = function()
  return "gen-comp-xmodem"
end
