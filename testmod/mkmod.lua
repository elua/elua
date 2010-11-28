require "pack"

local big_endian = false
local modname = "test"

local sign = 0x15AF29C8

local function put_number( n )
  return big_endian and string.pack( ">I", n ) or string.pack( "<I", n )
end

package.path = "../?.lua;" .. package.path

local p = require( "dynld.gcc_symbol_parser" )
local parser = p.new( "testmod.elf" )

-- Build a list of all the exported symbols
local f = io.open( "../dynld/symbol_list", "rb" )
local implist = {}
for l in f:lines() do
  local l2 = l:gsub( "%s+", "" )
  if #l2 > 1 and not l2:match( "^//" ) then
   if l2:sub( 1, 1 ) == '!' then l2 = l2:sub( 2 ) end
   implist[ l2 ] = true
  end
end  
f:close()

-- Write module name
local mod = modname .. "\0"

-- Write symbol table
for k, v in parser:iter() do 
  local name, address, isfunction, isglobal = unpack( v )
  if isfunction and isglobal and not implist[ name ] and name:find( "udl_" ) ~= 1 then
    mod = mod .. name .. "\0"
    while #mod % 4 ~= 0 do mod = mod .. "\0" end
    mod = mod .. put_number( address )
    print( string.format( "Added %s at offset %08X to symbol table", name, address ) )
  end
end
-- Write final '\0'
mod = mod .. "\0"

-- Align to 4 bytes
while #mod % 4 ~= 0 do mod = mod .. "\0" end

-- Get back and write signature and offset
mod = put_number( sign ) .. put_number( #mod + 8 ) .. mod

-- Append binary image
f = io.open( "udlimage.bin", "rb" )
mod = mod .. f:read( "*a" )
f:close()

-- Write it to file
local outf = io.open( "testmod.ebm", "wb" )
outf:write( mod )
outf:close()
print( string.format( "Module saved to testmod.ebm, size is %d bytes", #mod ) )

