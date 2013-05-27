-- Build configuration data

module( ..., package.seeall )
local sf = string.format
local utils = require "utils"

-- Helper: check a given option against a list of possibilities
local function validate_one( optv, optname, choices )
  if not optv then return true end
  if not utils.array_element_index( choices, optv ) then
    return nil, sf( "unknown %s '%s' in section 'build'", optname, optv )
  end
  return true
end

-- Validates the build arguments
function validate( desc, platform )
  local bd = desc.build
  if not bd then return true end
  
  -- Check allocator
  local res, err = validate_one( bd.allocator, 'allocator', { 'newlib', 'multiple', 'simple' } )
  if not res then return nil, err end
  -- Check target
  res, err = validate_one( bd.target, 'target', { 'lua', 'lualong', 'lualonglong' } )
  if not res then return nil, err end
  -- Check optram
  if bd.optram then
    if type( bd.optram ) ~= "boolean" then return false, "attribute 'optram' of section 'build' must be 'true' or 'false'" end
  end
  -- Check boot
  res, err = validate_one( bd.boot, 'boot', { 'standard', 'luarpc' } )
  if not res then return nil, err end
  -- Check romfs
  res, err = validate_one( bd.romfs, 'romfs', { 'verbatim', 'compress', 'compile' } )
  if not res then return nil, err end
  -- Check bootloader
  if bd.bootloader and platform ~= "avr32" then
    return false, "attribute 'bootloader' in section 'build' can only be specified for AVR32 targets"
  end
  res, err = validate_one( bd.bootloader, 'bootloader', { 'none', 'emblod' } )
  if not res then return nil, err end
  -- Check cpumode
  if bd.cpumode and platform ~= "arm" then
    return false, "attribute 'cpumode' in section 'build' can only be specified for ARM targets"
  end
  res, err = validate_one( bd.cpumode, 'cpumode', { 'arm', 'thumb' } )
  if not res then return nil, err end
  -- All done
  return true
end

