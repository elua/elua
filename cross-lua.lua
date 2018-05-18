local args = { ... }
local b = require "utils.build"
local builder = b.new_builder( ".build/cross-lua" )
local utils = b.utils
local sf = string.format
builder:init( args )
builder:set_build_mode( builder.BUILD_DIR_LINEARIZED )

local suffix = utils.is_windows() and '.exe' or ''
local output = 'luac.cross' .. suffix
local cdefs = '-DLUA_CROSS_COMPILER'

-- Lua source files and include path
local lua_files = [[lapi.c lcode.c ldebug.c ldo.c ldump.c lfunc.c lgc.c llex.c lmem.c lobject.c lopcodes.c
   lparser.c lstate.c lstring.c ltable.c ltm.c lundump.c lvm.c lzio.c lauxlib.c lbaselib.c
   ldblib.c liolib.c lmathlib.c loslib.c ltablib.c lstrlib.c loadlib.c linit.c luac.c print.c lrotable.c]]
lua_files = lua_files:gsub( "\n" , "" )
local lua_full_files = utils.prepend_path( lua_files, "src/lua" )
local local_include = "-Isrc/lua -Iinc/desktop -Iinc"

-- Compiler/linker options
builder:set_compile_cmd( sf( "gcc -O2 %s -Wall %s -c $(FIRST) -o $(TARGET)", local_include, cdefs ) )
builder:set_link_cmd( "gcc -o $(TARGET) $(DEPENDS) -lm" )

if not utils.is_dir( ".build" ) then
  if not utils.full_mkdir( ".build" ) then
    print( "[builder] Unable to create directory .build" )
    os.exit( 1 )
  end
end

-- Build everything
builder:make_exe_target( output, lua_full_files )
builder:build()

