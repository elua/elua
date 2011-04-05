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

compgen.enable = function( self, mode )
  -- Helper: check if a component is properly configured
  local function is_configured()
    if #self.options == 0 then return true end
    -- Iterate through all the component options and check them as needed
    for k, o in pairs( self.options ) do
      if not o:get_value() then return false end
      if not o:validate( o:get_value() ) then return false end
    end
    return self:validate_conf()
  end
  -- Disable always works
  self.enabled = false
  if not mode then return true end
  -- Enable needs a working configuration
  if not is_configured() then return false end
  self.enabled = true
  return true
end

compgen.validate_conf = function( self )
  return true
end

compgen.load_conf = function( self, t )
  -- Read the configuration from the given table
  if t.is_enabled == nil then
    error( sf( "Invalid configuration table in load_conf for component '%s'", self:get_name() ) )
  end
   if ( t.is_enabled and not self:can_enable() ) or ( not t.is_enabled and not self:can_disable() ) then
    error( sf( "Cannot enable/disable component '%s' in load_conf", self:get_name() ) )
  end
  -- Read all values (if any) from t
  for k, o in pairs( self.options ) do
    if t[ o:get_name() ] then
      local res = o:set_value( t[ o:get_name() ] )
      -- [TODO] check return of set_value, signal it to the user somehow
    end
  end
  self:enable( t.is_enabled )
end

compgen.get_conf = function( self )
  -- Write the whole configuration in a table
  local t = {}
  for k, o in pairs( self.options ) do
    t[ o:get_name() ] = o:get_value()
  end
  t.is_enabled = self:is_enabled() or false
  return t
end

-- Generic generator
compgen.generate = function( self, dest )
  dest:write( "// " .. self:get_help() .. "\n" )
  if self:get_enable_macro() then
    dest:write( "#define " .. self:get_enable_macro():upper() .. "\n" )
  end
  for k, o in pairs( self.options ) do
    local v = tostring( o:get_gen_value() )
    if not ( o:is_optional() and o:is_empty( v ) ) then
      o:get_macro_def( self.strout, dest )
    end
  end
  dest:write( "\n" )
end

compgen.__type = function()
  return "gen-comp"
end

-------------------------------------------------------------------------------
-- Functions that can be used by all instances

-- Create and return a timer choice option
compgen.option_timers = function( name, help, default )
  default = default or "Timer0"
  local tmrgen = assert( compgen.find_generator( "gen-io-timer" ) )
  local vtmrgen = assert( compgen.find_generator( "gen-comp-vtmr" ) )
  -- Return a choicemap that also includes the virtual timers if applicable
  local tmrmap = {}
  for i = 0, tmrgen:get_num() - 1 do
    table.insert( tmrmap, { text = "Timer" .. i, value = tostring( i ) } )
  end
  for i = 0, vtmrgen:get_num_vtimers() - 1 do
    table.insert( tmrmap, { text = "VTMR" .. i, value = sf( "( VTMR_FIRST_ID + %d )", i ) } )
  end
  return opt.choicemap_option( name, default, help, { values = tmrmap } )
end

-- Check how a timer option was modified
compgen.check_timer_change = function( self, name, help, state )
  -- Re-create the option and compare it with the previous one
  local crt_timer_opt = self:get_option_object( name )
  local crt_val = crt_timer_opt:get_value()
  local valid = not( state == "disabled" and crt_val and crt_val:find( "VTMR" ) == 1 )
  local action = function()
    local new_timer_opt = self.option_timers( name, help )
    self:add_option( new_timer_opt )
    if ( not valid and crt_val ) then new_timer_opt:set_value( new_timer_opt:get_default() ) end
  end
  local notif = ( not valid and crt_val ) and sf( "Warning: timer '%s' no longer present, switching to default value %s", crt_val, tostring( crt_timer_opt:get_default() ) )
  self:set_notification( notif, action )
end

-- Create and return a flow control choice option
compgen.option_flow_control = function( name, help, default )
  local flow_mapping =
  {
    { text = "None", value = "PLATFORM_UART_FLOW_NONE" },
    { text = "RTS", value = "PLATFORM_UART_FLOW_RTS" },
    { text = "CTS", value = "PLATFORM_UART_FLOW_CTS" },
    { text = "RTSCTS", value = "( PLATFORM_UART_FLOW_RTS | PLATFORM_UART_FLOW_CTS )" }
  }
  default = default or 'None'
  return opt.choicemap_option( name, default, help, { values = flow_mapping } )
end

