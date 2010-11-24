-- Build a symbol table for the eLua binary image
-- Also build the "driver" for the dynamic modules

require "pack"

local big_endian = false

local function put_number( s, n )
  return big_endian and s .. string.pack( ">I", n ) or s .. string.pack( "<I", n )
end

local p = require( "dynld.gcc_symbol_parser" )
local parser = p.new( "elua_lua_stm32f103re.elf" )
local g = require( "dynld.cortex_m3_stub_generator" )
local gen = g.new( "cortex_m3_stubs.s" )
local f = io.open( "dynld/symbol_list", "rb" )
local outdata = ""
local addr
local nsym = 1
for l in f:lines() do
  local l2 = l:gsub( "%s+", "" )
  if #l2 > 1 and not l2:match( "^//" ) then
    if l2:sub( 1, 1 ) == '!' then
      l2 = l2:sub( 2 )
      print( string.format( "Skipping lookup for obsolete symbol %s", l2 ) )
      addr = 0
    else 
      addr = parser:lookup( l2 )
      if not addr then
        print( string.format( "WARNING: symbol %s not found", l2 ) )
        addr = 0
      end
    end
    outdata = put_number( outdata, addr ) 
    gen:add_function( l2, addr, addr > 0 and nsym or 0 )
    nsym = nsym + 1   
  end
end
f:close()
gen:finalize()
f = io.open( "symaddr.bin", "wb" )
f:write( outdata )
f:close()
print( string.format( "Processed %d symbols", nsym - 1 ) )
