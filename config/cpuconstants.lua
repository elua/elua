-- CPU constants generator

module( ..., package.seeall )
local sf = string.format
local gen = require "generators"

function gen_constants( desc )
  local ct = desc.cpu_constants
  if not ct then return '' end
  local gstr = string.rep( "/", 80 ) .. "\n" .. "// Configured CPU constants\n\n"
  -- There are two types of macros in cpu_constants: simple (that are already defined somewhere)
  -- and full ((macro, def) pairs). We need to iterate over the full list first and define those
  -- macros. After that we generate the whole PLATFORM_CPU_CONSTANTS_CONFIGURED macro
  for _, m in pairs( ct ) do
    if type( m ) == "table" then
      local name, val = m[ 1 ], m[ 2 ]
      gstr = gstr .. "\n#ifndef " .. name .. "\n"
      gstr = gstr .. gen.print_define( name, val )
      gstr = gstr .. "#endif\n\n"
    end
  end

  gstr = gstr .. "#define PLATFORM_CPU_CONSTANTS_CONFIGURED\\\n"
  for _, m in pairs( ct ) do
    gstr = gstr .. sf( "  _C( %s ),\\\n", type( m ) == "string" and m or m[ 1 ] )
  end

  gstr = gstr .. "\n"
  return gstr:gsub( "\n\n\n", "\n\n" )
end

