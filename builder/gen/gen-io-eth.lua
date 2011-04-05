-- Ethernet generator
-- (bogus component, for dependency checking only)

module( ..., package.seeall )
local sf = string.format
local cgen = require "gen-io-simple"

-------------------------------------------------------------------------------
-- Public interface

function new( ctable )
  local t = cgen.new( ctable, "eth", "Ethernet" )
end
