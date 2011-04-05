-- Telnet component generator

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
  base.init_instance( self, 'telnet', ptable )
  self:set_enable_macro( 'BUILD_CON_TCP' )
  self:set_friendly_name( 'Telnet server' )
  return self
end

-- Extra dependency checking
cgen.check_explicit_deps = function( self )
  local cgen = assert( self.find_generator( self.COMP_CONSOLE ) )
  if cgen:is_enabled() then
    return false, "Telnet support and serial console can't be enabled at the same time"
  end
  return true
end

-- Initialization function
cgen.init = function( self, t )
  base.init( self, t )
  self:add_deps( self.COMP_TCP )
  self:set_help( 'The telnet server makes it possible to use the eLua shell and the Lua shell over a TCP/IP connection using telnet.' )
end

cgen.__type = function()
  return "gen-comp-telnet"
end
