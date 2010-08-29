-- Build a symbol table for the eLua binary image
-- Also build the "driver" for the dynamic modules

require "pack"

local big_endian = false

local function put_number( s, n )
  return big_endian and s .. string.pack( ">L", n ) or s .. string.pack( "<L", n )
end

local p = require( "dl.gcc_symbol_parser" )
local parser = p.new( "elua.map" )
local f = io.open( "dl/symbol_list", "rb" )
local outdata = ""
for l in f:lines() do
  local l2 = l:gsub( "%s+", "" )
  if #l2 > 1 and not l2:match( "^//" ) and not l2:match("OBSOLETE%s*$") then
    local addr = parser:lookup( l2 )
    if not addr then
      print( string.format( "WARNING: symbol %s not found", l2 ) )
    else
      outdata = put_number( outdata, addr )
    end
  end
end
f:close()
print( #outdata )
print( outdata )

