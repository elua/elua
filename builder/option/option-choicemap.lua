-- Option type: choicemap

module( ..., package.seeall )
local sf = string.format
local ioption = require "ioption"
local utils = require "utils"

choicemapoption = {}
local base = ioption.ioption
setmetatable( choicemapoption, { __index = base } )

new = function( name, default, help, args )
  local self = {}
  setmetatable( self, { __index = choicemapoption } )
  self:init_instance( name, default, help, args )
  return self
end

-- Choicemap validator
-- Args: 'values' for the list of choices, 'case' ('check' or 'ignore', default 'ignore')
-- to check
choicemapoption._validate = function( self, value )
  local a = self.args
  local pass = function( s ) return s end
  local filter = utils.tget( a, "case", "string" ) == "check" and pass or string.lower
  local nvalue = filter( value )
  for k, v in pairs( a.values ) do
    if filter( v.text ) == nvalue then return true, value end
  end
  return false
end

-- Range to string for choicemap
choicemapoption.get_range_string = function( self )
  local s = ""
  for k, v in pairs( self.args.values ) do
    s = s .. v.text .. "/"
  end
  return s:sub( 1, -2 )
end

choicemapoption.get_gen_value = function( self )
  local value = self.gen_value or self.value
  for k, v in pairs( self.args.values ) do
    if v.text == value then
      value = v.value
      break
    end
  end
  return value
end

choicemapoption.get_range_table = function( self )
  local vals = {}
  utils.foreach( self.args.values, function( k, v ) table.insert( vals, v.text ) end )
  return vals
end

choicemapoption.__type = function()
  return "choicemapoption"
end
