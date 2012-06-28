-- Code generators for various attributes and other constructs

module( ..., package.seeall )
local sf = string.format

local MACRO_DEF_POS = 41

-- Formatted print for "#define"
function print_define( k, v )
  v = v or ''
  local s = sf( "#define %s", k:upper() )
  if v then
    if #s < MACRO_DEF_POS then s = s .. string.rep( ' ', MACRO_DEF_POS - #s ) end
  end
  s = s .. v .. "\n"
  return s
end

-- Simple generator for an attribute
function simple_gen( attrname, conf, gentable )
  if gentable[ attrname ] then return '' end
  if not conf[ attrname ] then return '' end
  local adesc, aval = conf[ attrname ].desc, conf[ attrname ].value
  gentable[ attrname ] = true
  return print_define( attrname, aval )
end

