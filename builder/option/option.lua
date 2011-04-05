-- Different type of configuration options

module( ..., package.seeall )

local numoption = require "option-number"
local fixedoption = require "option-fixed"
local stringoption = require "option-string"
local choiceoption = require "option-choice"
local choicemapoption = require "option-choicemap"
local ipoption = require "option-ip"

-------------------------------------------------------------------------------
-- Public interface

function int_option( name, default, help, args )
  args = args or {}
  args.numtype = "integer"
  return numoption.new( name, default, help, args )
end

function float_option( name, default, help, args )
  args = args or {}
  args.numtype = "float"
  return numoption.new( name, default, help, args )
end

function choice_option( name, default, help, args )
  return choiceoption.new( name, default, help, args )
end

function choicemap_option( name, default, help, args )
  return choicemapoption.new( name, default, help, args )
end

function string_option( name, default, help, args )
  return stringoption.new( name, default, help, args )
end

function fixed_option( name, default, help, args )
  return fixedoption.new( name, default, help, args )
end

function ip_option( name, default, help, args )
  return ipoption.new( name, default, help, args )
end
