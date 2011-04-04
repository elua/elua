-- Timer (TMR) generator

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
  base.init_instance( self, 'timer', ctable )
  self:init( ctable )
  self:set_friendly_name( "timers" )
  return self
end

-- Initialization function
cgen.init = function( self, t )
  t = base.init( self, t )
end

-- Generator function
cgen.generate = function( self, dest )
  base.generate( self, dest, false )
end

cgen.__type = function()
  return "gen-io-timer"
end

-------------------------------------------------------------------------------
-- Public interface

function new( ctable )
  return cgen.new( ctable )
end

