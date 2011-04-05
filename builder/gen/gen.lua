-- Generator interface for the builder components

module( ..., package.seeall )
local sf = string.format
local utils = require "utils"

gentable = {}
gentable.MACROLEN = 42
gentable.genarray = {}
gentable.abstract = utils.abstract
gentable.notification_actions = {}

-------------------------------------------------------------------------------
-- Public list of all their generators and their names

-- First I/O subsysetms
gentable.IO_TIMER = 'gen-io-timer'
gentable.IO_UART = 'gen-io-uart'
gentable.IO_PIO = 'gen-io-pio'
gentable.IO_PWM = 'gen-io-pwm'
gentable.IO_ETH = 'gen-io-eth'
gentable.IO_SPI = 'gen-io-spi'

-- Then components
gentable.COMP_VTMR = 'gen-comp-vtmr'
gentable.COMP_CONSOLE = 'gen-comp-console'
gentable.COMP_XMODEM = 'gen-comp-xmodem'
gentable.COMP_TERM = 'gen-comp-term'
gentable.COMP_LINENOISE = 'gen-comp-linenoise'
gentable.COMP_ROMFS = 'gen-comp-romfs'
gentable.COMP_TCPIP = 'gen-comp-tcpip'
gentable.COMP_TELNET = 'gen-comp-telnet'
gentable.COMP_DHCPC = 'gen-comp-dhcpc'
gentable.COMP_DNSC = 'gen-comp-dnsc'
gentable.COMP_MMCFS = 'gen-comp-mmcfs'
gentable.COMP_SHELL = 'gen-comp-shell'

-------------------------------------------------------------------------------
-- Generator interface implementation

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
  self.optlist = {}
  self.enabler_macro = nil
  self.last_notification_res = ""
  self.notification_actions = {}
  self.is_io = false
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
  self:abstract()
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

gentable.add_option = function( self, option )
  local pos = #self.options + 1
  -- Is the option already added? If so, replace it
  for k, v in pairs( self.optlist ) do
    if option:get_name() == v.object:get_name() then
      pos = v.position
      break
    end
  end
  self.options[ pos ] = option
  self.optlist[ option:get_name() ] = { object = option, position = pos }
end

gentable.get_option_object = function( self, optname )
  return utils.tget( self.optlist[ optname ], "object", "table" )
end

gentable.get_option_position = function( self, optname )
  return utils.tget( self.optlist[ optname ], "position", 'number' )
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

-- Extra dependency checking
gentable.check_explicit_deps = function( self )
  return true
end

gentable.get_explicit_deps_string = function( self )
  return self.explicit_deps_string
end

gentable.set_explicit_deps_string = function( self, s )
  self.explicit_deps_string = s
end

-- Get the state of the component
gentable.get_state = function( self )
  return self.enabled and "enabled" or "disabled"
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

-- Enabler macro accesors
gentable.set_enable_macro = function( self, m )
  self.enabler_macro = m
end

gentable.get_enable_macro = function( self )
  return self.enabler_macro
end

gentable.is_io_generator = function( self )
  return self.is_io
end

-- Notifications on component state changes
gentable.notification = function( self, component, state )
end

gentable.get_last_notification = function( self )
  local t = self.last_notification_res
  self.last_notification_res = ""
  return t
end

gentable.execute_notification_actions = function( simulate )
  utils.foreach( gentable.notification_actions, function( k, v ) if not simulate then v() end end )
  gentable.notification_actions = {}
end

gentable.set_notification = function( self, res, action )
  if res then
    local c = "\n"
    if #self.last_notification_res == 0 or self.last_notification_res:sub( -1, -1 ) == "\n" then c = "" end
    self.last_notification_res = self.last_notification_res .. c .. res
  end
  if action then table.insert( gentable.notification_actions, action ) end
end

-- Notify all component that a component was changed
-- (except for the changed component)
gentable.notify_all = function( name, state )
  utils.foreach( gentable.genarray, function( k, c )
    if c.name ~= name then
      c.gen:notification( name, state )
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

