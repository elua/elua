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
  base.init_instance( self, 'shell', ptable )
  self:set_enable_macro( 'BUILD_SHELL' )
  self:set_friendly_name( 'eLua shell' )
  self:set_explicit_deps_string( 'Depends on either the serial console or telnet support' )
  return self
end

-- Check if either the serial console of the telnet support becomes disabled
cgen.notification = function( self, component, state )
  local gen
  if self:get_state() ~= "enabled" or state ~= "disabled" then return end
  if component == self.COMP_CONSOLE then gen = assert( self.find_generator( self.COMP_CONSOLE ) ) end
  if component == self.COMP_TELNET then gen = assert( self.find_generator( self.COMP_TELNET ) ) end
  if not gen then return end
  self:set_notification( sf( "Disabling component '%s' will automatically disable component '%s'", gen:get_friendly_name(), self:get_friendly_name() ),
    function() self:enable( false ) end )
end

-- Extra dependency checking
cgen.check_explicit_deps = function( self )
  local console_gen = assert( self.find_generator( self.COMP_CONSOLE ) )
  local telnet_gen = assert( self.find_generator( self.COMP_TELNET ) )
  if not console_gen:is_enabled() and not telnet_gen:is_enabled() then
    return false, "The shell needs either the serial console or telnet support"
  end
  return true
end

-- Initialization function
cgen.init = function( self, t )
  base.init( self, t )
  self:set_help( 'The eLua shell lets the user execute a number of system-related commands and run the Lua interpreter.' )
end

cgen.__type = function()
  return "gen-comp-shell"
end
