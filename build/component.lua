-- This module contains the generic functions for describing
-- an eLua build component

module( ..., package.seeall )

local sf = string.format
local ct = require "constants"
local at = require "attributes"
local gen = require "generators"

local components = {}
local conf = {}
local enabled = {}

-------------------------------------------------------------------------------
-- Various helper functions

-- Enables and configures the given component
local function config( name, data )
  local desc = components[ name ]
  local attrs = desc.attrs or {}
  -- Process each element in 'data' in turn
  for attr, v in pairs( data ) do
    local elmeta = attrs[ attr ]
    if not elmeta then error( sf( "attribute '%s' is not defined for component '%s'", attr, name ) ) end
    if elmeta.validator then
      local res, err = elmeta:validator( attr, v, name )
      if not res then
        error( sf( "'%s' is not a valid value for attribute '%s' of component '%s'", v, attr, name ) )
      else
        -- The validator can also change the attribute's value
        v = res
      end
    end
    if conf[ elmeta.macro ] and tostring( conf[ elmeta.macro ].value ) ~= tostring( v ) then
      print( sf( "WARNING: overriding value of '%s' from '%s' to '%s' with component '%s'", elmeta.macro,
        conf[ elmeta.macro ].value, v, name ) )
     end
    conf[ elmeta.macro ] = { desc = elmeta, value = v }
    -- print( sf( "SET -> '%s' = '%s'", elmeta.macro, v ) )
  end
  -- Set default values where needed
  for name, data in pairs( attrs ) do
    if not conf[ data.macro ] and data.default then
      conf[ data.macro ] = { desc = data, value = data.default }
      -- print( sf( "DEFAULT -> '%s' = '%s'", data.macro, data.default ) )
    end
  end
  -- Mark this component as configured
  enabled[ name ] = true
  -- print( sf( "ENABLED -> %s", name ) )
end

-------------------------------------------------------------------------------
-- Configuration checkers

-- Helper: shell configuration consistency checker
-- Returns 'true' if consistent, (false, errmsg) otherwise
local function shell_confcheck( compdesc, vals )
  if vals._SHELL_TRANSPORT.value == 'serial' then
    -- Check for required CON_UART_ID/CON_TIMER_ID/CON_UART_SPEED/CON_UART_FLOW
    if not vals.CON_UART_ID then
      return false, "required attribute 'uart' not specified for component 'shell'"
    elseif not vals.CON_UART_SPEED then
      return false, "required attribute 'speed' not specified for component 'shell'"
    end
  elseif vals._SHELL_TRANSPORT.value == 'tcpip' then
    if not enabled[ 'tcpip' ] then
      return false, "shell over TCP/IP can't be enabled if the 'tcpip' component is not enabled"
    end
  else
    return false, sf( "invalid value '%s' for attribute 'transport' of component 'shell'", vals._SHELL_TRANSPORT.value )
  end
  return true
end

-------------------------------------------------------------------------------
-- Component-specific generators

-- Generator for the shell component
local function shell_gen( desc, conf, generated )
  generated._SHELL_TRANSPORT = true
  local gstr = ''
  local shtype = conf._SHELL_TRANSPORT.value
  if shtype == 'serial' then
    gstr = gstr .. gen.simple_gen( "CON_UART_ID", conf, generated )
    gstr = gstr .. gen.simple_gen( "CON_UART_SPEED", conf, generated )
    gstr = gstr .. gen.simple_gen( "CON_TIMER_ID", conf, generated )
    gstr = gstr .. gen.simple_gen( "CON_FLOW_TYPE", conf, generated )
  else
    error "TODO: implement shell over TCP/IP generator"
  end
  return gstr
end

-------------------------------------------------------------------------------
-- Public interface

-- Build all components needed by eLua, save them in the "components" table
function init()
  -- Serial console
  components.sercon = { 
    macro = 'BUILD_CON_GENERIC',
    attrs = {
      uart = at.int_attr( 'CON_UART_ID' ),
      speed = at.int_attr( 'CON_UART_SPEED' ),
      timer = at.timer_attr( 'CON_TIMER_ID' ),
      flow = at.flow_control_attr( 'CON_FLOW_TYPE' )
    }
  }
  -- TCP/IP console
  components.tcpipcon = { macro = 'BUILD_CON_TCP' }
  -- XMODEM
  components.xmodem = {
    macro = 'BUILD_XMODEM',
    attrs = {
      uart = at.int_attr( 'CON_UART_ID' ),
      speed = at.int_attr( 'CON_UART_SPEED' ),
      timer = at.timer_attr( 'CON_TIMER_ID' ),
      flow = at.flow_control_attr( 'CON_FLOW_TYPE')
    }
  }
  -- Shell
  components.shell = {
    macro = 'BUILD_SHELL',
    confcheck = shell_confcheck,
    gen = shell_gen,
    attrs = {
      transport = at.choice_attr( '_SHELL_TRANSPORT', { 'serial', 'tcpip' }, 'serial' ),
      uart = at.int_attr( 'CON_UART_ID' ),
      speed = at.int_attr( 'CON_UART_SPEED' ),
      timer = at.timer_attr( 'CON_TIMER_ID' ),
      flow = at.flow_control_attr( 'CON_FLOW_TYPE' )
    }
  }
  -- Term
  components.term = {
    macro = 'BUILD_TERM',
    attrs = {
      uart = at.int_attr( 'CON_UART_ID' ),
      speed = at.int_attr( 'CON_UART_SPEED' ),
      timer = at.timer_attr( 'CON_TIMER_ID' ),
      flow = at.flow_control_attr( 'CON_FLOW_TYPE' ),
      lines = at.int_attr( 'TERM_LINES' ),
      cols = at.int_attr( 'TERM_COLS' )
    }
  }
  -- C interrupt support
  components.cints = { macro = 'BUILD_C_INT_HANDLERS' }
  -- Lua interrupt support
  components.luaints = {
    macro = 'BUILD_LUA_INT_HANDLERS',
    attrs = {
      queue_size = at.int_log2_attr( 'PLATFORM_INT_QUEUE_LOG_SIZE', nil, nil, 32 )
    }
  }
  -- Linenoise
  components.linenoise = { 
    macro = 'BUILD_LINENOISE',
    attrs = {
      shell_lines = at.int_attr( 'LINENOISE_HISTORY_SIZE_SHELL' ),
      lua_lines = at.int_attr( 'LINENOISE_HISTORY_SIZE_LUA' ),
      autosave_file = at.make_optional( at.string_attr( 'LINENOISE_AUTOSAVE_FNAME', '', 32 ) )
    }
  }
  -- RFS
  components.rfs = {
    macro = 'BUILD_RFS',
    attrs = {
      uart = at.int_attr( 'RFS_UART_ID' ),
      speed = at.int_attr( 'RFS_UART_SPEED' ),
      timer = at.timer_attr( 'RFS_TIMER_ID' ),
      flow = at.flow_control_attr( 'RFS_FLOW_TYPE' ),
      buf_size = at.int_log2_attr( 'RFS_BUFFER_SIZE', 512 ),
      timeout = at.int_attr( 'RFS_TIMEOUT', nil, nil, 100000 )
    }
  }
  -- ROMFS
  components.romfs = { macro = 'BUILD_ROMFS' }
  -- WOFS
  components.wofs = { macro = "BUILD_WOFS" }
end

-- Generate configuration data starting from the input dictionary
function gen_config( d )
  conf, enabled = {}, {}
  local comps = d.components

  -- Prerequisites: check for keys that might be needed in 'd', but might not be there
  -- At the moment, we need to definer either BUILD_CON_GENERIC or BUILD_CON_TCP (but not both)
  -- If none is defined, we default to BUILD_CON_GENERIC
  -- If both are defined, we exit with error
  if comps.sercon and comps.tcpipcon then
    return nil, "serial and TCP/IP console can't be enabled at the same time"
  elseif not comps.sercon and not comps.tcpipcon then
    comps.sercon = true
  end

  -- Step 1: interpret data in the input table
  if not comps then return false, "unable to find components in the board description" end
  -- Configure each component in turn, doing validation as required
  for compname, compval in pairs( comps ) do
    if not components[ compname ] then error( sf( "unknown component '%s'", compname ) ) end
    if type( compval ) ~= "table" and compval then
      comps[ compname ] = {}
      compval = comps[ compname ]
    end
    if compval then config( compname, compval ) end
  end

  -- Step 2: basic consistency check
  -- For each component, we check that all its required attributes (the ones that don't have
  -- an 'optional' key set to true) have a value. A component can overwrite this default 
  -- verification by specifying its own 'confcheck' function
  for compname, _ in pairs( enabled ) do
    local desc = components[ compname ]
    local attrs = desc.attrs or {}
    if desc.confcheck then
      local d, err = desc:confcheck( conf )
      if not d then return false, err end
    else
      for attr, adesc in pairs( attrs ) do
        if not conf[ adesc.macro ] and not adesc.optional then
          return false, sf( "required attribute '%s' of component '%s' not specified", attr, compname )
        end
      end
    end
  end

  -- Step 3: actual generation of code
  -- The default generator simply adds '#define KEY VALUE' pairs. A component can overwrite this
  -- default generation by specifying its own 'gen' function
  -- Also, we never generate the same key twice. We ensure this by keeping a table of the
  -- keys that were already generated
  local generated = {}
  local genstr = ''
  for compname, _ in pairs( enabled ) do
    local desc = components[ compname ]
    local attrs = desc.attrs or {}
    genstr = genstr .. sf( "// Configuration for component '%s'\n", compname )
    if desc.gen then
      genstr = genstr .. desc:gen( conf, generated )
    else
      for aname, adesc in pairs( attrs ) do 
        genstr = genstr .. gen.simple_gen( adesc.macro, conf, generated )
      end
    end
    -- Add the "build enable" macro
    if desc.macro then genstr = genstr .. gen.print_define( desc.macro ) .. "\n" end
  end

  -- Step 4: take care of all "special cases" (if any)
  -- TODO: check proper sermux ID assignment for console/RFS

  -- All done
  return genstr
end

