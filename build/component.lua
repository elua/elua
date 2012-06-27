-- This module contains the generic functions for describing
-- an eLua build component

module( ..., package.seeall )

local sf = string.format
local ct = require "constants"
local components = {}
local conf = {}
local enabled = {}

local MACRO_DEF_POS = 41

-- Enables and configures this component
local function config( name, data )
  local desc = components[ name ]
  local attrs = desc.attrs or {}
  -- Process each element in 'data' in turn
  for attr, v in pairs( data ) do
    local elmeta = attrs[ attr ]
    if not elmeta then error( sf( "attribute '%s' is not defined for component '%s'", attr, name ) ) end
    if elmeta.validator then
      if not elmeta:validator( v ) then
        error( sf( "'%s' is not a valid value for attribute '%s' of component '%s'", v, attr, name ) )
      end
    end
    if conf[ elmeta.macro ] then
      print( sf( "WARNING: overriding value of '%s' from '%s' to '%s' with component '%s'", elmeta.macro,
        conf[ elmeta.macro ].value, v, name ) )
     end
    conf[ elmeta.macro ] = { desc = elmeta, value = v }
    print( sf( "SET -> '%s' = '%s'", elmeta.macro, v ) )
  end
  -- Set default values where needed
  for name, data in pairs( attrs ) do
    if not conf[ name ] and data.default then
      conf[ data.macro ] = { desc = data, value = data.default }
      print( sf( "DEFAULT -> '%s' = '%s'", data.macro, data.default ) )
    end
  end
  -- Mark this component as configured
  enabled[ name ] = true
  print( sf( "ENABLED -> %s", name ) )
end

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

-- Returns a new timer option with the given macro and default (systmr if not specified)
local function timer_attr( macro, default )
  default = default or ct.systmr
  return { macro = macro, default = default, gen = TODO_tmr_gen }
end

-- Formatted print for "#define"
local function print_define( k, v )
  v = v or ''
  local s = sf( "#define %s", k:upper() )
  if v then
    if #s < MACRO_DEF_POS then s = s .. string.rep( ' ', MACRO_DEF_POS - #s ) end
  end
  s = s .. v .. "\n"
  return s
end

-- Simple generator for an attribute
local function simple_gen( attrname )
  local adesc, aval = conf[ attrname ].desc, conf[ attrname ].value
  return print_define( attrname, aval )
end

-------------------------------------------------------------------------------
-- Public interface

-- Build all components needed by eLua, save them in the "components" table
function init()
  -- XMODEM
  components.xmodem = {
    macro = 'BUILD_XMODEM',
    attrs = 
    {
      uart = { macro = 'CON_UART_ID' },
      speed = { macro = 'CON_UART_SPEED' },
      timer = timer_attr( 'CON_TIMER_ID' ),
      flow = { macro = 'CON_FLOW_TYPE', default = ct.uart_flow.none }
    }
  }
  -- Shell
  components.shell = {
    macro = 'BUILD_SHELL',
    confcheck = shell_confcheck,
    gen = shell_gen,
    attrs = 
    {
      transport = { macro = '_SHELL_TRANSPORT', default = 'serial' },
      uart = { macro = 'CON_UART_ID' },
      speed = { macro = 'CON_UART_SPEED' },
      timer = timer_attr( 'CON_TIMER_ID' ),
      flow = { macro = 'CON_FLOW_TYPE', default = ct.uart_flow.none }
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

  -- Step 1: interpret data in the input table
  local comps = d.components
  if not comps then return false, "unable to find components in the board description" end
  -- Configure each component in turn, doing validation as required
  for compname, compval in pairs( comps ) do
    if not components[ compname ] then error( sf( "unknown component '%s'", compname ) ) end
    if type( compval ) ~= "table" then
      comps[ compname ] = {}
      compval = comps[ compname ]
    end
    config( compname, compval )
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
  -- default verification by specifying its own 'gen' function
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
        if not generated[ adesc.macro ] then
          genstr = genstr .. simple_gen( adesc.macro )
          generated[ adesc.macro ] = true
        end
      end
    end
    -- Add the "build enable" macro
    genstr = genstr .. print_define( desc.macro ) .. "\n"
  end

  -- Step 4: take care of all "special cases"

  -- All done
  return genstr
end

