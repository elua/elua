-- Code generators for various attributes and other constructs

module( ..., package.seeall )
local sf = string.format

local MACRO_DEF_POS = 41

-- Formatted print for "#define"
function print_define( k, v )
  local s = sf( "#define %s", k )
  if v then
    if #s < MACRO_DEF_POS then s = s .. string.rep( ' ', MACRO_DEF_POS - #s ) end
  else
    v = ''
  end
  if type( v ) == "table" then
    s = s .. "{ " .. table.concat( v, "," ) .. " }" .. "\n"
  else
    s = s .. tostring( v ) .. "\n"
  end
  return s
end

-- Simple generator for an attribute
function simple_gen( attrname, conf, gentable )
  if gentable[ attrname ] then return '' end
  if not conf[ attrname ] then return '' end
  local adesc, aval = conf[ attrname ].desc, conf[ attrname ].value
  gentable[ attrname ] = true
  if adesc.is_ip then -- special generation for this one
    local s = ''
    for i = 1, 4 do
      s = s .. print_define( sf( "%s%d", attrname, i - 1 ), aval[ i ] )
    end
    return s
  end
  if adesc.mapping then aval = adesc.mapping[ aval ] end
  if adesc.is_array then
    -- The element is an array. The default is to define its value as { elements }
    aval = "{ " .. table.concat( aval, "," ) .. " }"
  end
  return print_define( attrname, tostring( aval ) )
end

