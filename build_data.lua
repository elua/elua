-- eLua build data
-- This contains various build time information:
--   supported toolchains
--   supported platforms and CPUs

module( ..., package.seeall )
local utils = require "utils"

-------------------------------------------------------------------------------
-- Build data

-- List of toolchains
local toolchain_list =
{
  [ 'arm-gcc' ] = {
    compile = 'arm-elf-gcc',
    link = 'arm-elf-ld',
    asm = 'arm-elf-as',
    bin = 'arm-elf-objcopy',
    size = 'arm-elf-size',
    cross_cpumode = 'little',
    cross_lua = 'float_arm 64',
    cross_lualong = 'int 32',
    version = '--version'
  },
  [ 'arm-eabi-gcc' ] = {
    compile = 'arm-eabi-gcc',
    link = 'arm-eabi-ld',
    asm = 'arm-eabi-as',
    bin = 'arm-eabi-objcopy',
    size = 'arm-eabi-size',
    cross_cpumode = 'little',
    cross_lua = 'float 64',
    cross_lualong = 'int 32',
    version = '--version'
  },
  codesourcery = {
    compile = 'arm-none-eabi-gcc',
    link = 'arm-none-eabi-ld',
    asm = 'arm-none-eabi-as',
    bin = 'arm-none-eabi-objcopy',
    size = 'arm-none-eabi-size',
    cross_cpumode = 'little',
    cross_lua = 'float 64',
    cross_lualong = 'int 32',
    version = '--version'
  },
  [ 'avr32-gcc' ] = {
    compile = 'avr32-gcc',
    link = 'avr32-ld',
    asm = 'avr32-as',
    bin = 'avr32-objcopy',
    size = 'avr32-size',
    cross_cpumode = 'big',
    cross_lua = 'float 64',
    cross_lualong = 'int 32',
    version = '--version'
  },
  [ 'avr32-unknown-none-gcc' ] = {
    compile = 'avr32-unknown-none-gcc',
    link = 'avr32-unknown-none-ld',
    asm = 'avr32-unknown-none-as',
    bin = 'avr32-unknown-none-objcopy',
    size = 'avr32-unknown-none-size',
    cross_cpumode = 'big',
    cross_lua = 'float 64',
    cross_lualong = 'int 32',
    version = '--version'
  },
  [ 'i686-gcc' ] = {
    compile = 'i686-elf-gcc',
    link = 'i686-elf-ld',
    asm = 'nasm',
    bin = 'i686-elf-objcopy',
    size = 'i686-elf-size',
    cross_cpumode = 'little',
    cross_lua = 'float 64',
    cross_lualong = 'int 32',
    version = '--version'
  }
}

-- Toolchain Aliases
toolchain_list[ 'devkitarm' ] = toolchain_list[ 'arm-eabi-gcc' ]

-- List of acrhitectures and their endianness
local arch_data = { 
  arm = 'little',
  cortexm = 'little',
  avr32 = 'big',
  i386 = 'little' 
}

-- Toolchain to arch mapping
local toolchain_map = {
  arm = { 'arm-gcc', 'codesourcery', 'devkitarm', 'arm-eabi-gcc' },
  cortexm = { 'arm-gcc', 'codesourcery', 'devkitarm', 'arm-eabi-gcc' },
  avr32 = { 'avr32-gcc', 'avr32-unknown-none-gcc' },
  i386 =  { 'i686-gcc' }
}

-- List of platform/CPU combinations
local platform_list =
{
  at91sam7x = { cpus = { 'AT91SAM7X256', 'AT91SAM7X512' }, arch = 'arm' },
  lm3s = { cpus = { 'LM3S1968', 'LM3S8962', 'LM3S6965', 'LM3S6918', 'LM3S9B92', 'LM3S9D92' }, arch = 'cortexm' },
  str9 = { cpus = { 'STR912FAW44' }, arch = 'arm' },
  i386 = { cpus = { 'I386' }, arch = 'i386' },
  sim = { cpus = { 'LINUX' }, arch = 'i386' },
  lpc288x = { cpus = { 'LPC2888' }, arch = 'arm' },
  str7 = { cpus = { 'STR711FR2' }, arch = 'arm' },
  stm32f2 = { cpus = { 'STM32F205RF' }, arch = 'cortexm' },
  stm32 = { cpus = { 'STM32F103ZE', 'STM32F103RE', 'STM32F103VCT6' }, arch = 'cortexm' },
  stm32f4 = { cpus =  { 'STM32F401RE', 'STM32F407VG', 'STM32F407ZG' }, arch = 'cortexm' },
  avr32 = { cpus = { 'AT32UC3A0128', 'AT32UC3A0256', 'AT32UC3A0512', 'AT32UC3B0256' }, arch = 'avr32' },
  lpc23xx = { cpus = { 'LPC2368' }, arch = 'arm' },
  lpc24xx = { cpus = { 'LPC2468' }, arch = 'arm' },
  lpc17xx = { cpus = { 'LPC1768', 'LPC1769' }, arch = 'cortexm' },
  xmc4000 = { cpus = { 'XMC4400F100X512', 'XMC4500F144K1024', 'XMC4500E144K1024', 'XMC4700F144K2048', 'XMC4300F100K256' }, arch = 'cortexm' },
}

-- Returns the platform of a given CPU
function get_platform_of_cpu( cpu )
  for p, v in pairs( platform_list ) do
    if utils.array_element_index( v.cpus, cpu:upper() ) then return p end
  end
end

-- Return all the CPUs in the 'platform_list' table
function get_all_cpus()
  local t = {}
  for pl, desc in pairs( platform_list ) do
    for _, cpu in pairs( desc.cpus ) do
      if not utils.array_element_index( t, cpu ) then t[ #t + 1 ] = cpu end
    end
  end
  return t
end

-- Returns the complete list of toolchains
function get_all_toolchains()
  local t = {}
  for arch, chains in pairs( toolchain_map ) do
    for _, cname in pairs( chains ) do
      if not utils.array_element_index( t, cname ) then t[ #t + 1 ] = cname end
    end
  end
  return t
end

-- Returns the list of toolchains for a given platform
function get_toolchains_of_platform( platform )
  return toolchain_map[ platform_list[ platform ].arch ]
end

-- Returns the data of the given toolchain
function get_toolchain_data( name )
  return toolchain_list[ name ]
end

-- Returns the endianness of the given platform
function get_endianness_of_platform( platform )
  return arch_data[ platform_list[ platform ].arch ]
end

