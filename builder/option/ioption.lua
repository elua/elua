-- Generic ioption interface

module( ..., package.seeall )
ioption = {}
local utils = require "utils"
local sf = string.format
ioption.abstract = utils.abstract

ioption.init_instance = function( self, name, default, help, args )
  self.args = args or {}
  self.name = name
  self.help = help
  self.default = default
  self.optional = utils.tget( args, "optional", "boolean" ) or false
end

ioption.get_default = function( self )
  return self.default
end

ioption.set_default = function( self, default )
  self.default = default
end

ioption.get_type = function( self )
  return self:__type()
end

ioption.set_gen_value = function( self, genv )
  return self.gen_value
end

ioption.set_value = function( self, value )
  if not self:validate( value ) then return end
  self.value = value
  return true
end

ioption.get_gen_value = function( self )
  return self.gen_value or self.value
end

ioption.get_value = function( self )
  return self.value
end

ioption.get_help = function( self )
  return self.help
end

ioption.get_name = function( self )
  return self.name
end

ioption.set_args = function( self, args )
  self.args = args
end

ioption.is_optional = function( self )
  return self.optional
end

ioption.is_empty = function( self, value )
  return false
end

-- Validate an option
ioption.validate = function( self, value )
  local valid, v = true, self.default
  if value ~= "" then
    valid, v = self:_validate( value )
  end
  return valid, v
end

ioption._validate = function( self, value )
  self:abstract()
end

-- Return the range of this ioption as a string
ioption.get_range_string = function( self )
  self:abstract()
end

-- Return the range of this ioption as a table
ioption.get_rangle_table = function( self )
  self:abstract()
end

-- Return the C macro definition of this ioption
ioption.get_macro_def = function( self, outfunc, dest )
  outfunc( dest, sf( "#define %s", self:get_name() ), tostring( self:get_gen_value() ) .. "\n" )
end
