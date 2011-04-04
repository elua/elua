-- Command line component configurator

module( ..., package.seeall )
local utils = require "utils"
local iui = require "ui"
local opt = require "option"
local sf = string.format
local tmpfile = "/tmp/.eluabuilder.data"

if utils.is_windows() then
  error "The 'dialog' configuration interface is only available under Windows"
end

if utils.check_command( "dialog --version" ) ~=0 then
  error "Cannot execute 'dialog'"
end

local dlgui = {}
setmetatable( dlgui, { __index = iui.uitable } )

dlgui.new = function()
  local self = {}
  setmetatable( self, { __index = dlgui } )
  return self
end

-- Show an "yes/no" prompt until a valid response is received
dlgui.get_yesno = function( self, prompt, def )
  local o = self.yesno
  def = def or 'y'
  o:set_default( def )
  local args = sf( '--title "%s configuration" --yesno %s "\n%s" 18 50', self.component:get_component(), def == 'n' and '--defaultno' or '', prompt )
  local res = os.execute( "dialog " .. args )
  return res == 0 and 'y' or 'n'
end

-- Show a message box
dlgui.msg_box = function( self, message )
  local args = sf( '--title "%s configuration" --msgbox "\n%s" 18 50', self.component:get_component(), message )
  os.execute( "dialog " .. args )
end

-- Get the value of an option
dlgui.get_option = function( self, o )
  local valid, v
  if o:get_type() == "fixed" then
    self:msg_box( sf( "%s\n\n%s\n\nSet to '%s' (fixed value)", o:get_name(), o:get_help(), o:get_default() ) )
    return
  end
  local cmd = sf( 'dialog --title "%s configuration"', self.component:get_component() )
  if o:get_type() == "number" then
    cmd = cmd .. sf( ' --inputbox "\n%s\n\n%s\n%s\n" 18 50 %s 2>%s', o:get_name(), o:get_help(), o:get_range_string(), o:get_default(), tmpfile )
  end
  while true do
    -- Execute the dialog command
    local res = os.execute( cmd )
    if res ~= 0 then return end
    -- Read back data if needed
    local f = assert( io.open( tmpfile, "rb" ) )
    local data = f:read( "*a" )
    f:close()
    os.remove( tmpfile )
    valid, v = o:validate( data )
    if valid then break end
    self:msg_box( "Invalid value entered" )
  end
  return v
end

dlgui.configure = function( self, c )
  self.component = c
  -- If the component is already enabled, show a help message
  -- Otherwise ask the user if the component should be enabled or not
  if c:is_enabled() then
    self:msg_box( c:get_help() .. "\n\nComponent is already enabled because it is needed by " .. utils.table_values_string( self:get_enable_requesters(), "," ) )
  else
    local resp = self:get_yesno( c:get_help() .. "\n\nEnable this component?", 'y' )
    if resp == 'n' then return false end
  end
  c:enable( true )

  -- Enter a loop to get all the configuration values
  local opts = c:get_options()
  for i = 1, #opts do
    local o = opts[ i ]
    local v = self:get_option( o )
    if v == nil then return end
    o:set_value( v )
  end
  return true
end

-------------------------------------------------------------------------------
-- Public interface

function new_configurator()
  return dlgui.new()
end

