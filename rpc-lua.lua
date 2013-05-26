local utils = require "utils.utils"
local sf = string.format

local output = 'luarpc'
local cdefs = "LUA_CROSS_COMPILER LUA_RPC"

local lua_files = [[lapi.c lcode.c ldebug.c ldo.c ldump.c lfunc.c lgc.c llex.c lmem.c lobject.c lopcodes.c
   lparser.c lstate.c lstring.c ltable.c ltm.c lundump.c lvm.c lzio.c lauxlib.c lbaselib.c
   ldblib.c liolib.c lmathlib.c loslib.c ltablib.c lstrlib.c loadlib.c linit.c lua.c print.c lrotable.c]]
lua_files = lua_files:gsub( "\n", "" )
local lua_full_files = utils.prepend_path( lua_files, "src/lua" )
lua_full_files = lua_full_files .. " src/modules/luarpc.c src/modules/lpack.c src/modules/bitarray.c src/modules/bit.c src/luarpc_desktop_serial.c "
local local_include = "src/lua inc src/modules inc/desktop"

if utils.is_windows() then
  lua_full_files = lua_full_files .. " src/serial/serial_win32.c"
  cdefs = cdefs .. " WIN32_BUILD"
else
  lua_full_files = lua_full_files .. " src/serial/serial_posix.c src/linenoise_posix.c"
  cdefs = cdefs .. " LUA_USE_LINENOISE"
end

local p = c.program{ output .. ".temp", src = lua_full_files, needs = "math", incdir = local_include, defines = cdefs, odir = ".build/rpc-lua", flags = "-m32", libflags = "-m32", debug = true, optimize = "O2" }
default( utils.build_helper( output, p ) )

