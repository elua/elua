-- Command line component configurator

module( ..., package.seeall )
local utils = require "utils"
local iui = require "ui"
local opt = require "option"
local sf = string.format

local cmdui = {}
setmetatable( cmdui, { __index = iui.uitable } )

cmdui.new = function()
  local self = {}
  setmetatable( self, { __index = cmdui } )
  return self
end

-- Choose an element from a list of elements
cmdui.choice = function( self, choices, default, title, body )
  print ""
  table.insert( choices, "Exit" )
  if title then print( utils.col_red( title ) ) end
  if body then print( utils.col_blue( body ) ) end
  local idx = 1
  utils.foreach( choices, function( k, v ) 
    io.write( utils.col_red( "  " .. tostring( idx ) .. ". " ) )
    print( type( v ) == "table" and v.text or tostring( v ) )
    idx = idx + 1
  end )
  print ""
  while true do
    io.write( utils.col_blue( sf( "Choose an option (%s): ", tostring( default ) ) ) )
    local resp = io.stdin:read( "*l" )
    resp = #resp == 0 and default or resp
    resp = tonumber( resp )
    if resp and math.floor( resp ) == resp and resp >= 1 and resp <= #choices then 
      return resp == #choices and 'exit' or resp
    end
    print( utils.col_red( "Invalid choice, please try again" ) )
  end
end

-- The menu is the same as the choice in cmdline
cmdui.menu = cmdui.choice

-- Display an information 
cmdui.info = function( self, text, title )
  print ""
  if title then print( utils.col_white( title ) ) end
  print( utils.col_white( text ) )
end

-- Show an "yes/no" prompt until a valid response is received
cmdui.ask_yesno = function( self, text, title )
  if title then print( utils.col_red( title ) ) end
  print( text )
  while true do
    io.write( "Are you sure you want to continue? (y/n) [y]: " )
    resp = io.stdin:read( "*l" )
    if resp:lower() == 'y' then 
      return "y"
    elseif resp:lower() == 'n' then
      return "n"
    end
    print( utils.col_red( "Please enter 'y' or 'n'" ) )
  end
end

-- Get the value of an option
cmdui.get_option = function( self, o )
  local resp, v, valid
  print( utils.col_yellow( o:get_name() ) )
  if o:get_type() == "fixedoption" then
    io.write( utils.col_blue( sf( "  %s: %s (fixed value)\n", o:get_help(), tostring( o:get_value() ) ) ) )
    return o:get_value()
  end
  while true do
    io.write( sf( "  %s (%s) [%s]: ", o:get_help(), o:get_range_string(), tostring( o:get_value() or o:get_default() ) ) )
    resp = io.stdin:read( "*l" )
    if #resp == 0 then return o:get_value() or o:get_default() end
    valid, v = o:validate( resp )
    if valid then break end
    print( utils.col_red( "Invalid value entered" .. ( type( v ) == "string" and ": " .. v or "" ) ) )
  end
  return v
end

cmdui.configure = function( self, c )
  -- Print generic information (component name and help)
  print( sf( "%s: %s", utils.col_green( "COMPONENT" ), utils.col_red( c:get_component() ) ) )
  print( sf( "%s: %s", utils.col_green( "INFO     " ), utils.col_blue( c:get_help() ) ) )

  -- Enter a loop to get all the configuration values
  local opts = c:get_options()
  for i = 1, #opts do
    local o = opts[ i ]
    local v = self:get_option( o )
    o:set_value( v )
  end
  print ""
  return true
end

-------------------------------------------------------------------------------
-- Public interface

function new_ui()
  return cmdui.new()
end

