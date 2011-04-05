-- Generator for simple peripherals that expose only the "num" parameter

module( ..., package.seeall )
local sf = string.format
local iogen = require "gen-io"
local utils = require "utils"

local cgen = {}
local base = iogen.iogen
setmetatable( cgen, { __index = base } )

new = function( ctable, peripheral, friendly_name )
  local self = {}
  setmetatable( self, { __index = cgen } )
  self.peripheral = peripheral
  base.init_instance( self, peripheral, ctable )
  base.init( self )
  self:set_friendly_name( friendly_name or peripheral:upper() )
  return self
end

cgen.__type = function( self )
  return "gen-io-" .. self.peripheral
end

