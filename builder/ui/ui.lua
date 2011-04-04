-- Component configurator interface

module( ..., package.seeall )
local utils = require "utils"
local opt = require "option"

uitable = {}
uitable.abstract = utils.abstract
uitable.yesno = opt.choice_option( "yesno", "y", "", { values = { "y", "n" } } )

uitable.new = function()
  local self = {}
  setmetatable( self, { __index = uitable } )
  return self
end

uitable.choice = function( self, choices, default, title, body )
  self:abstract()
end

uitable.value = function( self, option )
  self:abstract()
end

uitable.info = function( self, text, title )
  self:abstract()
end

uitable.ask_yesno = function( self, text, title )
  self:abstract()
end

uitable.configure = function( self, c )
  self:abstract()
end
