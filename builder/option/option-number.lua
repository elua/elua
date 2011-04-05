-- Option type: number

module( ..., package.seeall )
local sf = string.format
local ioption = require "ioption"
local utils = require "utils"

numoption = {}
local base = ioption.ioption
setmetatable( numoption, { __index = base } )

new = function( name, default, help, args )
  local self = {}
  setmetatable( self, { __index = numoption } )
  self:init_instance( name, default, help, args )
  return self
end

-- Number validator
-- Args: 'min' for minimum, 'max' for maximum, 'func' for special validation,
-- 'numtype' ('integer' or 'float') for type validation (default 'float')
numoption._validate = function( self, value )
  if type( value ) == "string" then value = tonumber( value ) end
  if type( value ) ~= "number" then return false end
  local a = self.args
  if utils.tget( a, "numtype", "string" ) == "integer" and math.floor( value ) ~= value then return false end
  if a.func then return a.func( value, a ) end
  local intervals = a.intervals or { a }
  for _, i in pairs( intervals ) do
    if i.min and value < i.min then return false end
    if i.max and value > i.max then return false end
  end
  return true, value
end

numoption.set_value = function( self, value )
  if not self:validate( value ) then return end
  self.value = tonumber( value )
  return true
end

-- Range to string for number
numoption.get_range_string = function( self )
  local a = self.args
  if a.range then return a.range end
  local nfmt = a.numtype == "integer" and "%d" or "%f"
  local fmt = "", temp
  local intervals = a.intervals or { a }
  for idx = 1, #intervals do
    local i = intervals[ idx ]
    local modified = true
    if i.min and i.max then
      temp = nfmt .. " <= n <= " .. nfmt
      fmt = fmt .. sf( temp, i.min, i.max )
      return sf( fmt, i.min, i.max )
    elseif i.min then
      temp = "n >= " .. nfmt
      fmt = fmt .. sf( temp, i.min )
    elseif i.max then
      temp = "n <= " .. nfmt
      fmt = fmt .. sf( temp, i.max )
    else
      modified = false
    end
    if modified and idx ~= #intervals then fmt = fmt .. ", " end
  end
  return #fmt > 0 and fmt or sf( "any %s value", a.numtype )
end

numoption.__type = function()
  return "numoption"
end
