-- Generate a C configuration starting from a Lua description file
-- for an eLua board

module( ..., package.seeall )

package.path = package.path .. ";utils/?.lua;config/?.lua"

local comps = require "components"
local cfgs = require "configurations"
local ct = require "constants"
local gen = require "generators"
local utils = require "utils"
local bd = require "build_data"
local mgen = require "modules"
local cpuct = require "cpuconstants"
local sects = require "sections"

local components, configs

-------------------------------------------------------------------------------
-- Various helpers and internal functions

-- Generator for section 'components'
local function generate_components( data, plconf )
  local compdata = data.components or {}

  -- Prerequisites: check for keys that might be needed in components, but might not be there
  -- At the moment, we need to definer either BUILD_CON_GENERIC or BUILD_CON_TCP (but not both)
  -- If none is defined, we default to BUILD_CON_GENERIC
  -- If both are defined, we exit with error
  if compdata.sercon and compdata.tcpipcon then
    return nil, "serial and TCP/IP console can't be enabled at the same time in section 'components'"
  elseif not compdata.sercon and not compdata.tcpipcon then
    compdata.sercon = true
  end

  -- Configure section first
  local res, err = sects.configure_section( components, 'components', compdata )
  if not res then return false, err end
  -- Let the backend do its validation
  if plconf.pre_generate_section then
    res, err = plconf.pre_generate_section( components, 'components', compdata, conf, enabled )
    if not res then return false, err end
  end

  -- Automatically generate BUF_ENABLE_UART if needed (sermux, RFS, CON_BUF_SIZE )
  if sects.enabled.sermux or sects.enabled.rfs or sects.conf[ 'CON_BUF_SIZE' ] then
    compdata.uart_buffers = {}
    sects.enabled.uart_buffers = true
  end
  -- TODO: consistency checks (for example, check proper sermux/RFS/console ID assignment)

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

-- Default table for backend configuration data
-- If the platform doesn't have a boardconf.lua, this is what we're going to use
local default_platform_conf = {
  add_platform_components = function() end,
  add_platform_configs = function() end,
  get_platform_modules = function() end,
  pre_generate_section = function() return true end,
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

#ifndef CON_BUF_SIZE
#define CON_BUF_SIZE          0
#endif // #ifndef CON_BUF_SIZE

#ifndef SERMUX_FLOW_TYPE
#define SERMUX_FLOW_TYPE      PLATFORM_UART_FLOW_NONE
#endif

#ifndef CON_FLOW_TYPE
#define CON_FLOW_TYPE         PLATFORM_UART_FLOW_NONE
#endif

#ifndef CON_TIMER_ID
#define CON_TIMER_ID          PLATFORM_TIMER_SYS_ID
#endif

#ifdef ELUA_BOOT_RPC
  #ifndef RPC_UART_ID
    #define RPC_UART_ID     CON_UART_ID
  #endif

  #ifndef RPC_TIMER_ID
    #define RPC_TIMER_ID    PLATFORM_TIMER_SYS_ID
  #endif

  #ifndef RPC_UART_SPEED
    #define RPC_UART_SPEED  CON_UART_SPEED
  #endif
#endif // #ifdef ELUA_BOOT_RPC

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

]], cboardname, cboardname )
  local desc, err = dofile( fname )
  if not desc then return false, err end
  if not desc.cpu then return false, "cpu not specified in board configuration file" end

  -- Find and require the platform board configuration file
  local platform = bd.get_platform_of_cpu( desc.cpu )
  if not platform then return false, sf( "unable to find the platform of cpu '%s'", desc.cpu ) end
  local plconf
  if utils.is_file( utils.concat_path{ 'src', 'platform', platform, 'boardconf.lua' } ) then
    plconf = require( "src.platform." .. platform .. ".boardconf" )
    print( sf( "Found a backend build configuration file for platform %s", platform ) )
  else
    plconf = default_platform_conf
  end

  -- Read platform specific components/configs
  plconf.add_platform_components( components )
  plconf.add_platform_configs( configs )

  -- Do we need to include any configured headers?
  if type( desc.headers ) == "table" and #desc.headers > 0 then 
    for _, h in pairs( desc.headers ) do
      header = header .. "#include " .. h .. "\n"
    end
    header = header .. "\n"
  end

  -- Do we need to add definitions for any configured macros?
  if type( desc.macros ) == "table" and #desc.macros > 0 then
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

  -- Then configs
  gen, err = generate_config( desc, plconf )
  local multi_alloc = sects.conf.use_multiple_allocator
  if not gen then return false, err end
  header = header .. gen

  -- Now it's a good time to include the fixed sanity checks
  header = header .. sanity_code

  -- TODO: call a "global checker" function that handles both components and configs
  -- Example: check if vuarts are specified, but sermux is not enbled
  -- Check if vtimers are specified, but virtual timers are not enabled
  -- TODO: let the user specify an allocator in the config file? (and validate it)

  -- Generate module configuration
  gen, err = mgen.gen_module_list( desc, plconf, platform )
  if not gen then return false, err end
  header = header .. gen

  -- Generate additional CPU constants
  gen, err = cpuct.gen_constants( desc )
  if not gen then return false, err end
  header = header .. gen

  -- All done, write the header's footer
  header = header .. sf( "#endif // #ifndef __GENERATED_%s_H__\n", cboardname )

  -- Return the contents of the header, as well as the name of the CPU used by this
  -- board (this information is needed by the builder) and the type of the allocator
  -- that should be used for this board
  return { header = header, cpu = desc.cpu, multi_alloc = multi_alloc }
end

