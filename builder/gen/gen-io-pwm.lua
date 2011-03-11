-- PWM generator

module( ..., package.seeall )
local sf = string.format
local cgen = require "gen-io-simple"

-------------------------------------------------------------------------------
-- Public interface

function new( ctable )
  return cgen.new( ctable, "pwm" )
end

