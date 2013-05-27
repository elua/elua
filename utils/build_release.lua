-- Script to build all the required files for a release

package.path = "utils/?.lua;config/?.lua;" .. package.path
require "lfs"
local utils = require "utils"

local args = { ... }
local rel = args[ 1 ] or "_temp"

-- Get the full list of boards
local boardnames = utils.string_to_table( utils.get_files( "boards/known", function( fname ) return fname:match( "%.lua$" ) end ) )
local boards = {}
utils.foreach( boardnames, function( k, v ) boards[ #boards + 1 ] = ( ( utils.split_ext( v:sub( #"boards/known/" + 1, -1 ) ) ) ) end )

local function docmd( cmd )
  os.execute( cmd )
end

-- Now build all targets
utils.foreach( utils.string_to_table( utils.get_files( ".", function( fname ) return fname:match( "%.elf$" ) or fname:match( "%.hex" ) or fname:match( "%.bin$" ) end, true ) ),
  function( _, name ) os.remove( name ) end )
utils.rmdir_rec( "dist" )
lfs.mkdir( "dist" )
for i = 1, #boards do
  local b = boards[ i ]
  print( utils.col_magenta( "Generating image for board " .. b .. " ... " ) )
  local cmd = "lua build_elua.lua board=" .. b .. " output_dir=dist disp_mode=minimal prog"
  docmd( cmd .. " -c" )
  docmd( cmd )
  print ""
end

-- Change names according to release number
for f in lfs.dir( "dist/" ) do
  local attrs = lfs.attributes( "dist/" .. f )
  if attrs.mode == "file" and f:find( "^elua_" ) then
    if f:find( "%.elf$" ) then
      if f ~= "elua_lua_pc.elf" then os.remove( "dist/" .. f ) end
    else
      local newname = f:gsub( "^elua_", "elua" .. rel .. "_" )
      os.rename( "dist/" .. f, "dist/" .. newname )
    end
  end
end

print "Done"

