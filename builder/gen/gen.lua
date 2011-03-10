-- Generator interface for the builder components

module( ..., package.seeall )
local sf = string.format
local utils = require "utils"

gentable = {}
gentable.MACROLEN = 42

gentable.new = function( name )
  local self = {}
  setmetatable( self, { __index = gentable } )
  self:init_instance( name )
  return self
end

gentable.init_instance = function( self, name )
  self.name = name
  self.is_available = false
  self.is_enabled = false
end

gentable.abstract = function( self )
  error( sf( "Function '%s' does not have an implementation", debug.getinfo( 2, "n" ).name ) )
end

gentable.is_available = function( self )
  return self.is_available
end

gentable.enable = function( self, mode )
  self.is_enabled = true
end

gentable.is_enabled = function( self )
  return self.is_enabled
end

gentable.generate = function( self, dest )
  self:abstract()
end

gentable.strout = function( dest, s, ... )
  dest:write( utils.strpad( s, gentable.MACROLEN ) )
  local t = { ... }
  t = utils.linearize_array( t )
  utils.foreach( t, function( k, v ) dest:write( v ) end )
end

gentable.__type = function()
  return "gen"
end

-------------------------------------------------------------------------------
-- Public interface

function new_gen( name )
  return gentable.new( name )
end

