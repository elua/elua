-- Terminal component generator

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
  base.init_instance( self, 'term', ptable )
  self:set_friendly_name( 'ANSI terminal support' )
  self:set_enable_macro( 'BUILD_TERM' )
  return self
end

-- Initialization function
cgen.init = function( self, t )
  base.init( self, t )
  self:set_help( 'ANSI terminal support. It uses the same UART as the serial console.' )
  self:add_deps{ self.COMP_CONSOLE }
  self:add_option( opt.int_option( 'TERM_LINES', 25, 'Number of lines in the terminal', { min = 1 } ) )
  self:add_option( opt.int_option( 'TERM_COLS', 80, 'Number of columns in the terminal', { min = 1 } ) )
end

cgen.__type = function()
  return "gen-comp-term"
end
