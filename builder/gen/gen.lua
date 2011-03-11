-- Generator interface for the builder components

module( ..., package.seeall )
local sf = string.format
local utils = require "utils"

gentable = {}
gentable.MACROLEN = 42
gentable.genarray = {}

gentable.new = function( name )
  local self = {}
  setmetatable( self, { __index = gentable } )
  self:init_instance( name )
  return self
end

gentable.init_instance = function( self, name )
  self.name = name
  self.deps = {}
  self.options = {}
  self.enabled = false
  table.insert( gentable.genarray, { name = name, gen = self } )
  self.help = ''
end

gentable.abstract = function( self )
  error( sf( "Function '%s' does not have an implementation", debug.getinfo( 2, "n" ).name ) )
end

gentable.can_enable = function( self, mode )
  self:abstract()
end

gentable.enable = function( self, mode )
  self.is_enabled = mode
  return true
end

gentable.is_enabled = function( self )
  return self.enabled
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

gentable.add_deps = function( self, deps )
  table.insert( self.deps, { deps } )
  self.deps = utils.linearize_array( self.deps )
end

gentable.get_deps = function( self )
  return self.deps
end

gentable.add_options = function( self, options )
  table.insert( self.options, { options } )
  self.options = utils.linearize_array( self.options )
end

gentable.get_options = function( self )
  return self.options
end

gentable.set_help = function( self, help )
  self.help = help
end

gentable.get_help = function( self )
  return self.help
end

gentable.get_genarray = function()
  return gentable.genarray
end

gentable.find_generator = function( name )
  local g 
  utils.foreach( gentable.genarray, function( k, v ) if v.name == name then g = v.gen end end )
  return g
end

gentable.__type = function()
  return "gen"
end

-------------------------------------------------------------------------------
-- Public interface

function new_gen( name )
  return gentable.new( name )
end

