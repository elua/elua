-- Different type of configuration options

module( ..., package.seeall )

local option = {}
local utils = require "utils"

--[[
Option types:
  1. 'number': numbers with validation
  2. 'choice': simple choice list
  3. 'string': string with size validation
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
  return true
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
    if filter( v ) == nvalue then return true end
  end
  return false
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
  return true
end

local vtable = { number = option._number_validator, choice = option._choice_validator, string = option._string_validator } 

-- Validate an option
-- Yes, this could be made 'more objectual', but I don't think this is needed
option.validate = function( self, value )
  return vtable[ self.otype ]( self, value )
end

-------------------------------------------------------------------------------
-- Public interface

function int_option( name, default, help, args )
  args = args or {}
  args.numtype = "integer"
  return option.new( "number", name, default, help, args )
end

function float_option( name, default, help, args )
  return option.new( "number", name, default, help, args )
end

function choice_option( name, default, help, args )
  return option.new( "choice", name, default, help, args )
end

function string_option( name, default, help, args )
  return option.new( "string", name, default, help, args )
end

