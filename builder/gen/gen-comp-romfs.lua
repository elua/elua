-- ROMFS component generator

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
  base.init_instance( self, 'romfs', ptable )
  self:set_enable_macro( 'BUILD_ROMFS' )
  self:set_friendly_name( 'Read-only file system' )
  return self
end

-- Initialization function
cgen.init = function( self, t )
  base.init( self, t )
  self:set_help( 'Read-only file system in Flash, built at compile time' )
end

cgen.__type = function()
  return "gen-comp-romfs"
end
