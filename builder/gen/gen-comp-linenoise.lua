-- Linenoise component generator

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
  base.init_instance( self, 'linenoise', ptable )
  self:set_friendly_name( '(e)Lua shell history' )
  self:set_enable_macro( 'BUILD_LINENOISE' )
  return self
end

-- Initialization function
cgen.init = function( self, t )
  base.init( self, t )
  self:set_help( 'Linenoise provides command line history and basic line edit support in both the eLua shell and the Lua interactive interpreter.' )
  self:add_deps{ self.COMP_TERM }
  self:add_option( opt.int_option( 'LINENOISE_HISTORY_SIZE_LUA', 20, "The number of lines kept in history for the Lua interpreter. " ..
    "If history support in Lua is not needed define this as 0.", { min = 1 } ) )
  self:add_option( opt.int_option( 'LINENOISE_HISTORY_SIZE_SHELL', 20, 'The number of lines kept in history for the eLua shell. ' ..
    'If history support in the eLua shell is not needed, define this as 0.', { min = 1 } ) )
  self:add_option( opt.string_option( 'LINENOISE_AUTOSAVE_FNAME', "", 'The history will automatically be saved everytime ' ..
    'the Lua interpreter exits in the filename specified by this macro. This macro is optional; if it’s not defined, the history will not ' ..
    'be saved automatically.', { maxlen = 32, optional = true } ) )
end

cgen.__type = function()
  return "gen-comp-linenoise"
end
