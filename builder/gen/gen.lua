-- Generator interface for the builder components

module( ..., package.seeall )
local sf = string.format
local utils = require "utils"

gentable = {}
gentable.MACROLEN = 42
gentable.genarray = {}
gentable.abstract = utils.abstract

gentable.new = function( name )
  local self = {}
  setmetatable( self, { __index = gentable } )
  self:init_instance( name )
  return self
end

gentable.init_instance = function( self, name )
  assert( gentable.find_generator( name ) == nil, sf( "attempt to add generator '%s' more than once", name ) )
  self.name = name
  self.deps = {}
  self.options = {}
  self.enabled = false
  self.help = ''
  self.enable_requesters = {}
  self.disable_requesters = {}
  self.friendly_name = name
  self.last_notification_res = nil
  table.insert( gentable.genarray, { name = name, gen = self } )
end

gentable.abstract = function( self )
  error( sf( "Function '%s' does not have an implementation", debug.getinfo( 2, "n" ).name ) )
end

gentable.can_enable = function( self, mode )
  self:abstract()
end

gentable.can_disable = function( self, mode )
  self:abstract()
end

gentable.enable = function( self, mode )
  self.enabled = mode
  return true
end

gentable.is_enabled = function( self )
  return self.enabled
end

gentable.generate = function( self, dest )
  self:abstract()
end

gentable.is_configured = function( self )
  self:abstract()
end

gentable.set_configured = function( self, flag )
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

gentable.add_option = function( self, options )
  table.insert( self.options, options )
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

gentable.get_name = function( self )
  return self.name
end

gentable.get_gen_array = function()
  return gentable.genarray
end

gentable.find_generator = function( name )
  local g 
  utils.foreach( gentable.genarray, function( k, v ) if v.name == name then g = v.gen end end )
  return g
end

-- Get the state of the component
gentable.get_state = function( self )
  if self:is_configured() then
    return 'configured'
  elseif self:is_enabled() then
    return 'enabled'
  else
    return 'disabled'
  end
end    

gentable.set_friendly_name = function( self, s )
  self.friendly_name = s
end

gentable.get_friendly_name = function( self )
  return self.friendly_name
end

gentable.__type = function()
  return "gen"
end

gentable.notification = function( self, component, enabled )
end

gentable.get_notification_res = function( self )
  local t = self.last_notification_res
  self.last_notification_res = nil
  return t
end

gentable.set_notification_res = function( self, res )
  self.last_notification_res = res
end

-- Notify all component that a component was changed
-- (except for the changed component)
gentable.notify_all = function( name, enabled )
  utils.foreach( gentable.genarray, function( k, c )
    if c.name ~= name then
      c.gen:notification( name, enabled )
    end
  end )
end

-------------------------------------------------------------------------------
-- Public interface

function get_gen_array()
  return gentable.genarray
end

function find_generator( name )
  return gentable.find_generator( name )
end

