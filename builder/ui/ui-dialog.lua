-- Command line component configurator

module( ..., package.seeall )
local utils = require "utils"
local iui = require "ui"
local opt = require "option"
local sf = string.format
local tmpfile = "/tmp/.eluabuilder.data"
local dw, dh = 60, 25
local totalchoice = 10

local function nonl( s )
  return s:sub( -1, -1 ) == "\n" and s:sub( 1, -2 ) or s
end

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
dlgui.ask_yesno = function( self, prompt, title, def )
  def = def or 'y'
  title = title or "Confirmation"
  local args = sf( '--title "Confirmation" --yesno %s "\n%s" %d %d', def == 'n' and '--defaultno' or '', prompt, dh, dw )
  local res = os.execute( "dialog " .. args )
  return res == 0 and 'y' or 'n'
end

-- Show a message box
dlgui.info = function( self, message, title )
  title = title or "Configurator message"
  local args = sf( '--title "%s" --msgbox "\n%s" %d %d', title, message, dh, dw )
  os.execute( "dialog " .. args )
end

-- Choose an option from a menu
dlgui.menu = function( self, choices, default, title, body )
  table.insert( choices, "Exit" )
  local args = sf( '--title "%s" --menu "\n%s" %d %d %d ', title , body, dh, dw, #choices > totalchoice and totalchoice or #choices )
  utils.foreach( choices, function( k, v )
    args = args .. sf( '%d "%s" ', k, type( v ) == "table" and v.text or tostring( v ) )
  end )
  args = args .. sf( " 2>%s", tmpfile )
  local res = os.execute( "dialog " .. args )
  if res ~= 0 then return 'exit' end
  res = utils.read_file( tmpfile )
  res = tonumber( res )
  return res == #choices and 'exit' or res
end

-- Choose an element from a list of elements
dlgui._get_choice = function( self, choices, title, body, default )
  local args = sf( '--title "%s" --radiolist "\n%s" %d %d %d ', title, body, dh, dw, #choices > totalchoice and totalchoice or #choices )
  utils.foreach( choices, function( k, v )
    args = args .. sf( '%d "%s" %s ', k, type( v ) == "table" and v.text or tostring( v ), default == v and "on" or "off" )
  end )
  args = args .. sf( " 2>%s", tmpfile )
  local res = os.execute( "dialog " .. args )
  if res ~= 0 then return "" end
  return utils.read_file( tmpfile )
end

-- Get a string from the user
dlgui._get_string = function( self, title, body, default )
  local args = sf( '--title "%s" --inputbox "\n%s" %d %d %s 2>%s', title, body, dh, dw, tostring( default ), tmpfile )
  local res = os.execute( "dialog " .. args )
  return res ~= 0 and "" or utils.read_file( tmpfile )
end

-- Get the value of an option
dlgui.get_option = function( self, o )
  local valid, v
  if o:get_type() == "fixedoption" then
    self:info( sf( "%s\n\n%s\n\nSet to '%s' (fixed value)", o:get_name(), o:get_help(), o:get_default() ) )
    return o:get_value()
  end
  while true do
    if o:get_type() == "choicemapoption" or o:get_type() == "choiceoption" then
      local r = o:get_range_table()
      res = self:_get_choice( r, o:get_name(), o:get_help(), o:get_value() or o:get_default() )
      if #res == 0 then return o:get_value() or o:get_default() end
      return r[ tonumber( res ) ]
    else
      res = self:_get_string( o:get_name(), sf( "%s\n%s\n", o:get_help(), o:get_range_string() ), o:get_value() or o:get_default() )
      if #res == 0 then return o:get_value() or o:get_default() end
      valid, v = o:validate( res )
      if valid then break end
      self:info( "Invalid value entered" .. ( type( v ) == "string" and ": " .. v or "" ), "Configurator error" )
    end
  end
  return v
end

dlgui.configure = function( self, c )
  -- Print generic information (component name and help)
  --print( sf( "%s: %s", utils.col_green( "COMPONENT" ), utils.col_red( c:get_component() ) ) )
  --print( sf( "%s: %s", utils.col_green( "INFO     " ), utils.col_blue( c:get_help() ) ) )

  -- Enter a loop to get all the configuration values
  local opts = c:get_options()
  for i = 1, #opts do
    local o = opts[ i ]
    local v = self:get_option( o )
    o:set_value( v )
  end
  return true
end

-------------------------------------------------------------------------------
-- Public interface

function new_ui()
  return dlgui.new()
end

