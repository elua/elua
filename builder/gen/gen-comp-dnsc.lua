-- DNS client component generator

module( ..., package.seeall )
local sf = string.format
local compgen = require "gen-comp"
local utils = require "utils"
local opt = require "option"

local cgen = {}
local base = compgen.compgen
setmetatable( cgen, { __index = base } )

new = function( ptable )
  local self = {}
  setmetatable( self, { __index = cgen } )
  base.init_instance( self, 'dnsc', ptable )
  self:set_friendly_name( 'DNS client' )
  self:set_enable_macro( 'BUILD_DNS' )
  return self
end

-- Initialization function
cgen.init = function( self, t )
  base.init( self, t )
  self:set_help( 'Internet address resolver (DNS client) for eLua.' )
  self:add_deps{ self.COMP_TCPIP }
end

cgen.__type = function()
  return "gen-comp-dnsc"
end
