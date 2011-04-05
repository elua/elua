-- VTMR component generator

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
  base.init_instance( self, 'vtmr', ptable )
  self:set_friendly_name( 'Virtual timers' )
  return self
end

-- Initialization function
cgen.init = function( self, t )
  base.init( self, t )
  t = self.ptable
  self:set_help( 'Virtual timers (multiple software timers running on a single hardware timer interrupt handler)' )
  self:add_deps( self.IO_TIMER )
  self:add_option( opt.int_option( 'VTMR_NUM_TIMERS', 4, 'Number of virtual timers', { min = 1 } ) )
  self:add_option( opt.int_option( 'VTMR_FREQ_HZ', 10, 'Virtual timer frequency', { min = 1 } ) )
  -- VTMR_TIMER_ID: this is implemented using the SysTick timer on Cortex-M3 so it doesn't make
  -- sense adding a special option here. Or so I think.
  local allopts = {}
  local tmrgen = assert( self.find_generator( self.IO_TIMER ), "unable to find gen-io-timer" )
  local tmropt
  if utils.tget( t, "family", "string" ) == "cortex_m3" then
    self:add_option( opt.fixed_option( 'VTMR_TIMER_ID', 'SysTick', 'ID of the hardware timer used for virtual timers' ) )
  else
    self:add_option( opt.int_option( 'VTMR_TIMER_ID', 0, 'ID of the hardware timer used for virtual timers', { min = 0, max = tmrgen:get_num() - 1 } ) )
  end
end

-- Return the number of virtual timers
cgen.get_num_vtimers = function( self )
  if self:is_enabled() then
    local o = self:get_option_object( 'VTMR_NUM_TIMERS' )
    return o:get_value()
  else
    return 0
  end
end

cgen.__type = function()
  return "gen-comp-vtmr"
end

