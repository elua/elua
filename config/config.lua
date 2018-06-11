-- Generate a C configuration starting from a Lua description file
-- for an eLua board

module( ..., package.seeall )

package.path = "utils/?.lua;config/?.lua;" .. package.path

local comps = require "components"
local cfgs = require "configurations"
local gen = require "generators"
local utils = require "utils"
local bd = require "build_data"
local mgen = require "modules"
local cpuct = require "cpuconstants"
local sects = require "sections"
local bargs = require "buildargs"

local components, configs
local glconf, glen

-------------------------------------------------------------------------------
-- Various helpers and internal functions

-- Generator for section 'components'
local function generate_components( data, plconf )
  local compdata = data.components or {}

  -- Prerequisites: check for keys that might be needed in components, but might not be there
  -- At the moment, we need to definer either BUILD_CON_GENERIC or BUILD_CON_TCP (but not both)
  if compdata.sercon and compdata.tcpipcon then
    return nil, "serial and TCP/IP console can't be enabled at the same time in section 'components'"
  elseif not compdata.sercon and not compdata.tcpipcon then
    return nil, "either serial (sercon) or TCP/IP (tcpipcon) console must be enabled in 'components'"
  end

  -- Configure section first
  local res, err = sects.configure_section( components, 'components', compdata )
  if not res then return false, err end
  -- Let the backend do its validation
  if plconf.pre_generate_section then
    res, err = plconf.pre_generate_section( components, 'components', compdata, conf, enabled )
    if not res then return false, err end
  end

  -- Generate all data for section 'components'
  return sects.generate_section( components, 'components', compdata )
end

-- Generator for section 'config'
local function generate_config( data, plconf )
  local confdata = data.config or {}

  -- Configure section first
  local res, err = sects.configure_section( configs, 'config', confdata )
  if not res then return false, err end
  -- Let the backend do its validation
  if plconf.pre_generate_section then
    res, err = plconf.pre_generate_section( configs, 'config', confdata, conf, enabled )
    if not res then return false, err end
  end

  return sects.generate_section( configs, 'config', confdata )
end

-- Global sanity checks (data is in 'glconf' and 'glen'
local function check_components_and_config()
  -- Check all uart IDs. If VUARTs are specified but sermux is not enabled, return with error
  if not glen.sermux then
    for _, attrdata in pairs( glconf ) do
      local attrval = attrdata.value
      if attrdata.desc.is_uart and type( attrval ) == "string" and attrval:find( "^vuart" ) then
        return false, sf( "attribute '%s' of element '%s' in section '%s' reffers to a virtual UART, but the serial multiplexer ('sermux') is not enabled",
          attrdata.name, attrdata.elname, attrdata.sectname )
      end
    end
  end

  -- Check all timer IDs. If virtual timers are specified but vtmr is not enabled, return with error
  if not glen.vtmr or tostring( glconf.VTMR_NUM_TIMERS.value ) == "0" then
    for _, attrdata in pairs( glconf ) do
      local attrval = attrdata.value
      if attrdata.desc.is_timer and type( attrval ) == "string" and attrval:find( "^vtmr" ) then
        return false, sf( "attribute '%s' of element '%s' in section '%s' reffers to a virtual timer, but virtual timers ('vtmr') are not enabled",
          attrdata.name, attrdata.elname, attrdata.sectname )
      end
    end
  end

  -- Check sermux/RFS+console proper UART assignment
  if glen.sermux and glen.sercon and glen.rfs then
    local rfs_uart_value = tostring( glconf.RFS_UART_ID.value )
    local con_uart_value = tostring( glconf.CON_UART_ID.value )
    if rfs_uart_value:find( "^vuart" ) and rfs_uart_value ~= "vuart0" then
      io.write( utils.col_yellow( "[CONFIG] WARNING: you have enabled the serial multiplexer and RFS over a virtual serial port which is not the first virtual serial port ('vuart0')" ) )
      print( utils.col_yellow( "In this configuration, the serial multiplexer will not work in 'rfsmux' mode. Check the serial multiplexer section of the eLua manual for more details." ) )
    elseif rfs_uart_value == "vuart0" and con_uart_value:find( "^vuart" ) and con_uart_value ~= "vuart1" then
      io.write( utils.col_yellow( "[CONFIG] WARNING: when using both RFS and the serial console with 'sermux', it's best to set the serial console uart ID to the second virtual serial port ('vuart1')" ) )
      print( utils.col_yellow( "In this configuration, the serial multiplexer will work directly in 'rfsmux' mode with a console. Check the serial multiplexer section of the eLua manual for more details." ) )
    end
  end

  return true
end

-- Default table for backend configuration data
-- If the platform doesn't have a boardconf.lua, this is what we're going to use
local default_platform_conf = {
  add_platform_components = function() end,
  add_platform_configs = function() end,
  get_platform_modules = function() end,
  pre_generate_section = function() return true end,
}

-- Default table for extra configuration (same as above)
local default_extra_conf  = {
  get_extra_modules = function() end,
}

-- Sanity code
-- These are more checks added to the generated header file
-- Some of these are the result of pure paranoia. Nevertheless, they seem to work.

local sanity_code = [[
///////////////////////////////////////////////////////////////////////////////
// Static sanity checks and additional defines

#if defined( ELUA_BOOT_RPC ) && !defined( BUILD_RPC )
#define BUILD_RPC
#endif

#if defined( BUILD_LUA_INT_HANDLERS ) || defined( BUILD_C_INT_HANDLERS )
  #define BUILD_INT_HANDLERS

  #ifndef INT_TMR_MATCH
  #define INT_TMR_MATCH         ELUA_INT_INVALID_INTERRUPT
  #endif
#endif // #if defined( BUILD_LUA_INT_HANDLERS ) || defined( BUILD_C_INT_HANDLERS )

#ifndef VTMR_NUM_TIMERS
#define VTMR_NUM_TIMERS       0
#endif // #ifndef VTMR_NUM_TIMERS

#ifndef SERMUX_FLOW_TYPE
#define SERMUX_FLOW_TYPE      PLATFORM_UART_FLOW_NONE
#endif

#ifndef CON_FLOW_TYPE
#define CON_FLOW_TYPE         PLATFORM_UART_FLOW_NONE
#endif

#ifndef CON_TIMER_ID
#define CON_TIMER_ID          PLATFORM_TIMER_SYS_ID
#endif

#ifndef RFS_FLOW_TYPE
#define RFS_FLOW_TYPE         PLATFORM_UART_FLOW_NONE
#endif

#ifdef ELUA_BOOT_RPC
  #ifndef RPC_UART_ID
    #define RPC_UART_ID       CON_UART_ID
  #endif

  #ifndef RPC_TIMER_ID
    #define RPC_TIMER_ID      PLATFORM_TIMER_SYS_ID
  #endif

  #ifndef RPC_UART_SPEED
    #define RPC_UART_SPEED    CON_UART_SPEED
  #endif
#endif // #ifdef ELUA_BOOT_RPC

#if ( defined( BUILD_RFS ) || defined( BUILD_SERMUX ) || defined( CON_BUF_SIZE ) || defined ( CDC_BUF_SIZE ) ) && !defined( BUF_ENABLE_UART )
#define BUF_ENABLE_UART
#endif

#if defined( ADC_BUF_SIZE ) && !defined( BUF_ENABLE_ADC )
#define BUF_ENABLE_ADC
#endif

#ifndef CPU_FREQUENCY
#define CPU_FREQUENCY         0
#endif

]]

-------------------------------------------------------------------------------
-- Public interface

-- This code is executed an initialization time (the first 'require')
components = comps.init()
configs = cfgs.init()

-- Read the Lua description file of a board and return the corresponding header file
-- as a string or (false, error) if an error occured
function compile_board( fname, boardname )
  local cboardname = boardname:upper():gsub( "[%-%.%s]", "_" )
  local header = sf([[
// Lua board configuration file, automatically generated

#ifndef __GENERATED_%s_H__
#define __GENERATED_%s_H__

#include <stddef.h>

]], cboardname, cboardname )
  local desc, err = dofile( fname )
  if not desc then return false, err end
  if not desc.cpu then return false, "cpu not specified in board configuration file" end

  -- Check the keys in 'desc'
  local known_keys = { 'cpu', 'ps_modules_name', 'components', 'config', 'headers', 'macros', 'modules', 'cpu_constants', 'build' }
  for k, _ in pairs( desc ) do
    if not utils.array_element_index( known_keys, k ) then return false, sf( "unknown key '%s'", k ) end
  end

  -- Check CPU
  local cpulist = bd.get_all_cpus()
  if not utils.array_element_index( cpulist, desc.cpu:upper() ) then
    io.write( utils.col_red( "[CONFIG] Allowed CPUS: " ) )
    for i = 1, #cpulist do io.write( utils.col_red( cpu_list[ i ] .. " " ) ) end
    print ""
    return false, sf( "unknown cpu '%s'", desc.cpu )
  end

  -- Find and require the platform board configuration file
  local platform = bd.get_platform_of_cpu( desc.cpu )
  if not platform then return false, sf( "unable to find the platform of cpu '%s'", desc.cpu ) end
  local plconf = default_platform_conf
  if utils.is_file( utils.concat_path{ 'src', 'platform', platform, 'build_config.lua' } ) then
    plconf = require( "src.platform." .. platform .. ".build_config" )
  end

  -- Find and require the extra build configuration if specified
  local extraconf = default_extra_conf
  if utils.is_file( utils.concat_path{ comp.extras, 'build_config.lua' } ) then
    extraconf = require( comp.extras .. ".build_config" )
  end

  -- Read platform specific components/configs
  plconf.add_platform_components( components, boardname, desc.cpu )
  plconf.add_platform_configs( configs, boardname, desc.cpu )

  -- Do we need to include any configured headers?
  if type( desc.headers ) == "table" and #desc.headers > 0 then 
    for _, h in pairs( desc.headers ) do
      header = header .. "#include " .. h .. "\n"
    end
    header = header .. "\n"
  end

  -- Do we need to add definitions for any configured macros?
  if type( desc.macros ) == "table" and #desc.macros > 0 then
    header = header .. string.rep( "/", 80 ) .. "\n"
    header = header .. "// Configuration for section 'macros'\n\n"
    for _, m in pairs( desc.macros ) do
      if type( m ) == "string" then -- #define m
        header = header .. gen.print_define( m )
      elseif type( m ) == "table" then -- { macro, value } -> #define macro value
        header = header .. gen.print_define( m[ 1 ], m[ 2 ] )
      end
    end
    header = header .. "\n"
  end

  -- Generate components first
  local gen, err = generate_components( desc, plconf )
  if not gen then return false, err end
  header = header .. gen
  -- Keep generated data for later use
  glconf, glen = sects.conf, sects.enabled

  -- Then configs
  gen, err = generate_config( desc, plconf )
  local multi_alloc = cfgs.needs_multiple_allocator()
  if not gen then return false, err end
  header = header .. gen
  -- Accumulate generated data into 'glconf' and 'glen'
  utils.concat_tables( glconf, sects.conf )
  utils.concat_tables( glen, sects.enabled )

  -- Now we have all components and the configuration generated
  -- It's a good time for some sanity checks
  gen, err = check_components_and_config()
  if not gen then return false, err end

  -- Now it's a good time to include the fixed sanity checks
  header = header .. sanity_code

  -- Generate module configuration
  mgen.add_extra_modules( extraconf.get_extra_modules() )
  gen, err = mgen.gen_module_list( desc, plconf, platform, boardname )
  if not gen then return false, err end
  header = header .. gen

  -- Generate additional CPU constants
  gen, err = cpuct.gen_constants( desc )
  if not gen then return false, err end
  header = header .. gen

  -- All done, write the header's footer
  header = header .. sf( "#endif // #ifndef __GENERATED_%s_H__\n", cboardname )

  -- We are done with the header, we still need to check the compile flags
  gen, err = bargs.validate( desc, platform )
  if not gen then return false, err end

  -- Return the contents of the header, as well as the name of the CPU used by this
  -- board (this information is needed by the builder) and the build information
  return { header = header, cpu = desc.cpu, multi_alloc = multi_alloc, build = desc.build }
end

