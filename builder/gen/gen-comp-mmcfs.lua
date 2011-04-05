-- SD/MMC file system component generator

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
  base.init_instance( self, 'mmcfs', ptable )
  self:set_friendly_name( 'SD/MMC filesystem' )
  self:set_enable_macro( 'BUILD_MMCFS' )
  return self
end

-- Pin number validator
local function _validate_pin( value, args )
  local self = args.instance
  if value < 0 then return false, "invalid pin number" end
  local crt_port = self:get_option_object( 'MMCFS_CS_PORT' )
  local pio_gen = assert( self.find_generator( self.IO_PIO ) )
  assert( crt_port:get_value() )
  local maxpins = pio_gen:get_pins_in_port( crt_port:get_value() )
  if value >= maxpins then return false, sf( "invalid pin number for port %d", crt_port:get_value() ) end
  return true, value
end

-- Initialization function
cgen.init = function( self, t )
  base.init( self, t )
  self:set_help( 'SD/MMC R/W filesystem based on Elm Chan\'s FatFs.' )
  self:add_deps{ self.IO_SPI, self.IO_PIO }
  local spi_gen = assert( self.find_generator( self.IO_SPI ) )
  local pio_gen = assert( self.find_generator( self.IO_PIO ) )
  self:add_option( opt.int_option( 'MMCFS_TICK_HZ', 10, 'The rate at which SD/MMC timer function disk_timerproc() are being called by the platform.', { min = 1 } ) )
  self:add_option( opt.int_option( 'MMCFS_SPI_NUM', 0, 'ID of the SPI peripheral to be used by MMCFS', { min = 0, max = spi_gen:get_num() - 1 } ) )
  self:add_option( opt.int_option( 'MMCFS_CS_PORT', 0, 'ID of the port to which the MMC/SD card CS line is connected', { min = 0, max = pio_gen:get_num() - 1 } ) )
  self:add_option( opt.int_option( 'MMCFS_CS_PIN', 0, 'Pin to which the MMC/SD card CS line is connected', { instance = self, func = _validate_pin, range = "a valid pin number" } ) )
end

cgen.__type = function()
  return "gen-comp-mmcfs"
end
