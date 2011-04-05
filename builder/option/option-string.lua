-- Option type: string

module( ..., package.seeall )
local sf = string.format
local ioption = require "ioption"
local utils = require "utils"

stringoption = {}
local base = ioption.ioption
setmetatable( stringoption, { __index = base } )

new = function( name, default, help, args )
  local self = {}
  setmetatable( self, { __index = stringoption } )
  self:init_instance( name, default, help, args )
  return self
end

-- String validator
--- Args: 'minlen' to check minimum length, 'maxlen' to check maximum length,
--- 'func' for function validation
stringoption._validate = function( self, value )
  local a = self.args
  if type( value ) ~= "string" then return false end
  if a.minlen and #value < a.minlen then return false end
  if a.maxlen and #value > a.maxlen then return false end
  if a.func then return a.func( value, a ) end
  return true, value
end

-- Range to string for strings
stringoption.get_range_string = function( self )
  local a = self.args
  if a.minlen and a.maxlen then
    return sf( "min. %d chars, max. %d chars", a.minlen, a.maxlen )
  elseif a.minlen then
    return sf( "min. %d chars", a.minlen )
  elseif a.maxlen then
    return sf( "max. %d chars", a.maxlen )
  else
    return "any string"
  end
end

stringoption.is_empty = function( self, value )
  return value == ""
end

stringoption.get_macro_def = function( self, outfunc, dest )
  local v = sf( '"%s"', self:get_gen_value() )
  outfunc( dest, sf( "#define %s", self:get_name() ), v .. "\n" )
end

stringoption.__type = function()
  return "stringoption"
end
