local args = { ... }
local b = require "utils.build"
local builder = b.new_builder( ".build/rpc-lua" )
local utils = b.utils
local sf = string.format
builder:init( args )
builder:set_build_mode( builder.BUILD_DIR_LINEARIZED )

local output = 'luarpc'
local cdefs = "-DLUA_CROSS_COMPILER -DLUA_RPC"

local lua_files = [[lapi.c lcode.c ldebug.c ldo.c ldump.c lfunc.c lgc.c llex.c lmem.c lobject.c lopcodes.c
   lparser.c lstate.c lstring.c ltable.c ltm.c lundump.c lvm.c lzio.c lauxlib.c lbaselib.c
   ldblib.c liolib.c lmathlib.c loslib.c ltablib.c lstrlib.c loadlib.c linit.c lua.c print.c lrotable.c]]
lua_files = lua_files:gsub( "\n", "" )
local lua_full_files = utils.prepend_path( lua_files, "src/lua" )
lua_full_files = lua_full_files .. " src/modules/luarpc.c src/modules/lpack.c src/modules/bitarray.c src/modules/bit.c src/luarpc_desktop_serial.c "
local local_include = "-Isrc/lua -Iinc -Isrc/modules -Iinc/desktop"

if utils.is_windows() then
  lua_full_files = lua_full_files .. " src/serial/serial_win32.c"
  cdefs = cdefs .. " -DWIN32_BUILD"
else
  lua_full_files = lua_full_files .. " src/serial/serial_posix.c src/linenoise_posix.c"
  cdefs = cdefs .. " -DLUA_USE_LINENOISE "
end

-- Compiler/linker options
builder:set_compile_cmd( sf( "gcc -O2 -g %s -Wall %s -c $(FIRST) -o $(TARGET)", local_include, cdefs ) )
builder:set_link_cmd( "gcc -o $(TARGET) $(DEPENDS) -lm" )

-- Build everything
builder:make_exe_target( output, lua_full_files )
builder:build()

