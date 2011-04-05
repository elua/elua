-- Option type: fixed

module( ..., package.seeall )
local sf = string.format
local ioption = require "ioption"
local utils = require "utils"

fixedoption = {}
local base = ioption.ioption
setmetatable( fixedoption, { __index = base } )

new = function( name, default, help, args )
  local self = {}
  setmetatable( self, { __index = fixedoption } )
  self:init_instance( name, default, help, args )
  self:set_value( default )
  if type( args ) == "table" then self:set_gen_value( args.gen ) end
  return self
end

-- Fixed validator
fixedoption._validate = function( self, value )
  return true, self.default
end

-- Range to string for fixed
fixedoption.get_range_string = function( self )
  return tostring( self.default )
end

fixedoption.__type = function()
  return "fixedoption"
end
