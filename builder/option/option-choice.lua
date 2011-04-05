-- Option type: choice

module( ..., package.seeall )
local sf = string.format
local ioption = require "ioption"
local utils = require "utils"

choiceoption = {}
local base = ioption.ioption
setmetatable( choiceoption, { __index = base } )

new = function( name, default, help, args )
  local self = {}
  setmetatable( self, { __index = choiceoption } )
  self:init_instance( name, default, help, args )
  return self
end

-- Choice validator
-- Args: 'values' for the list of choices, 'case' ('check' or 'ignore', default 'ignore')
-- to check
choiceoption._validate = function( self, value )
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
choiceoption.get_range_string = function( self )
  return utils.table_values_string( self.args.values, "/" )
end

choiceoption.get_range_table = function( self )
  local vals = {}
  utils.foreach( self.args.values, function( k, v ) table.insert( vals, v ) end )
  return vals
end

choiceoption.__type = function()
  return "choiceoption"
end
