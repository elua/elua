-- Option type: IP

module( ..., package.seeall )
local sf = string.format
local ioption = require "ioption"
local utils = require "utils"

ipoption = {}
local base = ioption.ioption
setmetatable( ipoption, { __index = base } )

new = function( name, default, help, args )
  local self = {}
  setmetatable( self, { __index = ipoption } )
  self:init_instance( name, default, help, args )
  return self
end

-- IP option validator
ipoption._validate = function( self, value )
  if type( value ) ~= "string" then return false end
  local ndots = 0
  for i = 1, #value do
    local b = value:sub( i, i )
    if b == "." then
      ndots = ndots + 1
    elseif b:find( "%d" ) ~= 1 then
      return false
    end
  end
  if ndots ~= 3 then return false end
  local ncomp = 0
  for c in value:gmatch( "%d+" ) do
    if not tonumber( c ) then return false end
    local n = tonumber( c )
    if n < 0 or n > 255 then return false end
    ncomp = ncomp + 1
  end
  if ncomp ~= 4 then return false end
  return true, value
end

-- Range to string for strings
ipoption.get_range_string = function( self )
  return "IP address (xxx.yyy.zzz.ttt)"
end

ipoption.is_empty = function( self, value )
  return value == ""
end

ipoption.get_macro_def = function( self, outfunc, dest )
  local idx = 0
  local v = self:get_gen_value()
  for c in v:gmatch( "%d+" ) do
    outfunc( dest, sf( "#define %s%d", self:get_name(), idx ), tostring( c ) .. "\n" )
    idx = idx + 1
  end
end

ipoption.__type = function()
  return "ipoption"
end
