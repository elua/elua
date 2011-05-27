-- Script to build all the required files for a release
-- NOT FOR WINDOWS!

require "lfs"

local args = { ... }
local rel = args[ 1 ]

-- Board list (note that this must be synchronized manually with SConstruct)
local boards = 
{ 
  'SAM7-EX256', 'EK-LM3S1968', 'EK-LM3S8962', 'EK-LM3S6965',
  'EK-LM3S9B92', 'STR-E912', 'LPC-H2888',
  'MOD711', 'STM3210E-EVAL', 
  { board = 'ATEVK1100', options = '' },
  { board = 'ATEVK1101', options = '' },
  'ET-STM32', 'EAGLE-100', 'ELUA-PUC', 'MBED',
  { board = 'MIZAR32', options = "target=lualong" },
  { board = 'PC', options = "" }
}

local function docmd( cmd )
  print( cmd )
  os.execute( cmd )
end

-- Now build all targets
os.execute( "rm -f *.elf *.hex *.bin" )
os.execute( "rm -rf dist/" )
os.execute( "mkdir dist" )
for i = 1, #boards do
  local b = boards[ i ]
  local cmd = "scons  "
  if type( b ) == "string" then
    cmd = cmd .. "board=" .. b .. " toolchain=codesourcery prog "
    docmd( cmd .. "-c" )
    docmd( cmd .. "-j2" )
  else
    cmd = cmd .. "board=" .. b.board .. " " .. b.options .. " prog "
    docmd( cmd .. "-c" )
    docmd( cmd .. "-j2" )
  end
end
os.execute( "mv elua_lua_i386.elf dist/" )
os.execute( "rm -f *.elf" )
os.execute( "mv elua* dist/" )

-- Change names according to release number
for f in lfs.dir( "dist/" ) do
  local attrs = lfs.attributes( "dist/" .. f )
  if attrs.mode == "file" and f:find( "^elua_" ) then
    local newname = f:gsub( "^elua_", "elua" .. rel .. "_" )
    os.rename( "dist/" .. f, "dist/" .. newname )
  end
end

print "Done"

