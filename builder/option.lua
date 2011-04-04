-- Different type of configuration options

module( ..., package.seeall )

local option = {}
local utils = require "utils"
local sf = string.format

--[[
Option types:
  1. 'number': numbers with validation
  2. 'choice': simple choice list
  3. 'string': string with size validation
  4. 'fixed': fixed value (must be equal to the default)
--]]

option.new = function( otype, name, default, help, args )
  local self = {}
  setmetatable( self, { __index = option } )
  self.otype = otype
  self.args = args or {}
  self.name = name
  self.help = help
  self.default = default
  return self
end

option.get_default = function( self )
  return self.default
end

option.set_default = function( self, default )
  self.default = default
end

option.get_type = function( self )
  return self.otype
end

option.set_value = function( self, value )
  if not self:validate( value ) then return end
  self.value = self.args.otype == "number" and tonumber( value ) or value
  return true
end

option.get_value = function( self )
  return self.value
end

option.get_help = function( self )
  return self.help
end  

option.get_name = function( self )
  return self.name
end

-- Number validator
-- Args: 'min' for minimum, 'max' for maximum, 'func' for special validation,
-- 'numtype' ('integer' or 'float') for type validation (default 'float')
option._number_validator = function( self, value )
  if type( value ) == "string" then value = tonumber( value ) end
  if type( value ) ~= "number" then return false end
  local a = self.args
  if utils.tget( a, "numtype", "string" ) == "integer" and math.floor( value ) ~= value then return false end 
  if a.min and value < a.min then return false end
  if a.max and value > a.max then return false end
  if a.func then return a.func( value, a ) end
  return true, value
end

-- Range to string for number
option._number_range = function( self )
  local a = self.args
  local nfmt = a.numtype == "integer" and "%d" or "%f"
  local fmt
  if a.min and a.max then
    fmt = nfmt .. " <= n <= " .. nfmt
    return sf( fmt, a.min, a.max ) 
  elseif a.min then
    fmt = "n >= " .. nfmt
    return sf( fmt, a.min ) 
  elseif a.max then
    fmt = "n <= " .. nfmt
    return sf( fmt, a.max ) 
  else
    return sf( "any %s value", a.numtype )
  end
end

-- Choice validator
-- Args: 'values' for the list of choices, 'case' ('check' or 'ignore', default 'ignore')
-- to check
option._choice_validator = function( self, value )
  local a = self.args
  local pass = function( s ) return s end
  local filter = utils.tget( a, "case", "string" ) == "check" and pass or string.lower
  local nvalue = filter( value )
  for k, v in pairs( a.values ) do 
    if filter( v ) == nvalue then return true, value end
  end
  return false
end

-- Range to string for choice
option._choice_range = function( self )
  return utils.table_values_string( self.args.values, "/" )
end

-- String validator
-- Args: 'minlen' to check minimum length, 'maxlen' to check maximum length,
-- 'func' for function validation
option._string_validator = function( self, value )
  local a = self.args
  if type( value ) ~= "string" then return false end
  if a.minlen and #value < a.minlen then return false end
  if a.maxlen and #value > a.maxlen then return false end
  if a.func then return a.func( value, a ) end
  return true, value
end

-- Range to string for string
option._string_range = function( self )
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

-- Fixed validator
option._fixed_validator = function( self, value )
  return true, self.default
end

-- Range to string for fixed
option._fixed_range = function( self )
  return tostring( self.default )
end

local vtable = { number = option._number_validator, choice = option._choice_validator, string = option._string_validator,
  fixed = option._fixed_validator }
local rtable = { number = option._number_range, choice = option._choice_range, string = option._string_range, 
  fixed = option._fixed_range }

-- Validate an option
-- Yes, this could be made 'more objectual', but I don't think this is needed
option.validate = function( self, value )
  local valid, v = true, self.default
  if value ~= "" then 
    valid, v = vtable[ self.otype ]( self, value )
  end
  return valid, v
end

-- Return the range of this option as a string
option.get_range_string = function( self )
  return rtable[ self.otype ]( self )
end

-------------------------------------------------------------------------------
-- Public interface

function int_option( name, default, help, args )
  args = args or {}
  args.numtype = "integer"
  return option.new( "number", name, default, help, args )
end

function float_option( name, default, help, args )
  args = args or {}
  args.numtype = "float"
  return option.new( "number", name, default, help, args )
end

function choice_option( name, default, help, args )
  return option.new( "choice", name, default, help, args )
end

function string_option( name, default, help, args )
  return option.new( "string", name, default, help, args )
end

function fixed_option( name, default, help, args )
  return option.new( "fixed", name, default, help, args )
end

