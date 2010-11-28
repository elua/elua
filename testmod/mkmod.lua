require "pack"

local big_endian = false
local modsrcname = "testmod.c"
local modname
local sign = 0x15AF29C8

local function printf( fmt, ... )
  io.write( string.format( fmt, ... ) )
end

-- Parse a #define line from C
local function parse_define( l )
  l = l:gsub( "%s+", " " )
  if l:find( "#define" ) ~= 1 then return nil end
  l = l:gsub( "^%s*", "" )
  l = l:gsub( "//.*$", "" ) .. " "
  local _, __, ___, n, v = l:find( "(#.-)%s(.-)%s(.-)%s+" )
  if n and v and v:sub( 1, 1 ) == '"' and v:sub( -1, -1 ) == '"' then v = v:sub( 2, -2 ) end
  return n, v 
end

-- Look for UDL_MOD_NAME and UDL_MOD_VERSION in the source file
local cf = io.open( modsrcname, "rb" )
assert( cf )
for l in cf:lines() do
  local n, v = parse_define( l )
  if n and v then
    if n == "UDL_MOD_NAME" then
      modname = v
      break
    end
  end
end
cf:close()

if not modname then
  print "UDL_MOD_NAME not defined in the source file"
  return 1
end
if #modname > 15 then
  print "UDL_MOD_NAME must be maximum 15 chars in length"
  return 1
end
printf( "Module name is '%s'\n", modname )

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

-- Read binary image
f = io.open( "udlimage.bin", "rb" )
assert( f )
local modbin = f:read( "*a" )
f:close()

-- Write module name (always 16 bytes)
while #modname < 16 do modname = modname .. "\0" end
local mod = ""

-- Write symbol table
for k, v in parser:iter() do 
  local name, address, isfunction, isglobal = unpack( v )
  if isglobal and not implist[ name ] and name:find( "udl_" ) ~= 1 then
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

-- Structure: signature, size (symtable+data), data offset, name, symbol table, actual data
mod = put_number( sign ) .. put_number( #mod + #modbin ) .. put_number( #mod + 16 ) .. modname .. mod .. modbin

-- Write it to file
local outf = io.open( "testmod.ebm", "wb" )
outf:write( mod )
outf:close()
print( string.format( "Module saved to testmod.ebm, size is %d bytes", #mod ) )

