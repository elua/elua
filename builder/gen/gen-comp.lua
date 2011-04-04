-- Generator interface for the eLua components

module( ..., package.seeall )
local sf = string.format
local gen = require "gen"
local utils = require "utils"
local opt = require "option"

compgen = {}
local base = gen.gentable
setmetatable( compgen, { __index = base } )

compgen.new = function( component, ptable )
  local self = {}
  setmetatable( self, { __index = compgen } )
  self:init_instance( component, ptable )
  return self
end

compgen.init_instance = function( self, component, ptable )
  base.init_instance( self, 'gen-comp-' .. component )
  self.component = component
  self.ptable = ptable
end

compgen.get_component = function( self )
  return self.component
end

compgen.init = function( self, ptable )
  self.ptable = ptable or self.ptable
end

compgen.can_enable = function( self )
  return true
end

compgen.can_disable = function( self )
  return true
end

compgen.is_configured = function( self )
  -- Iterate through all the component options and check them as needed
  for k, o in pairs( self.options ) do
    if not o:get_value() then return false end
    if not o:validate( o:get_value() ) then return false end
  end
  return true
end

compgen.__type = function()
  return "gen-comp"
end


