-- PIO generator

module( ..., package.seeall )
local sf = string.format
local iogen = require "gen-io"
local utils = require "utils"

local cgen = {}
local base = iogen.iogen
setmetatable( cgen, { __index = base } )

cgen.new = function( ctable )
  local self = {}
  setmetatable( self, { __index = cgen } )
  base.init_instance( self, 'pio', ctable )
  self:init( ctable )
  return self
end

-- Initialization function
cgen.init = function( self, t )
  t = base.init( self, t )
  if not t then return end
  -- PIO tables need 'prefix' to be either 'number' or 'letter'
  local f = utils.tget( t, "prefix", "string" )
  if not f or ( f ~= "number" and f ~= "letter" ) then
    error "[gen-pio] 'prefix' must be specified as either 'number' or 'letter'"
  end
  self.prefix = f == 'number' and '0' or 'A'
  -- PIO tables can have either "pins_per_port" or "pin_array"
  f = utils.tget( t, "pins_per_port", "number" )
  if f then
   self.pins_per_port = f
  else
    f = utils.tget( t, "pin_array", "table" )
    if f then
      if #f ~= self.num then
        error( sf( "[gen-pio] pin_array table has length '%d', expected '%d'", #f, self.num ) )
      end
      self.pin_array = f
    else
      error( "[gen-pio] 'pins_per_port' or 'pin_array' must be specified in 'pio' table" )
    end
  end
end

-- Generator function
cgen.generate = function( self, dest )
  base.generate( self, dest, true )
  if self.is_available then
    self.strout( dest, "#define PIO_PREFIX", sf( "'%s'\n", self.prefix ) )
    if self.pins_per_port then
      self.strout( dest, "#define PIO_PINS_PER_PORT", sf( "%d\n", self.pins_per_port ) )
    else
      self.strout( dest, "#define PIO_PIN_ARRAY", "{ " )
      utils.foreach( self.pin_array, function( k, v ) dest:write( sf( "%d%s ", v, k == #self.pin.array and "" or "," ) ) end )
      dest:write( "}\n" )
    end
  end
  dest:write( "\n" )
end

cgen.__type = function()
  return "gen-pio"
end

-------------------------------------------------------------------------------
-- Public interface

function new( ctable )
  return cgen.new( ctable )
end

