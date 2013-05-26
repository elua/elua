local utils = require "utils.utils"
local sf = string.format

local output = 'luac.cross'
local cdefs = 'LUA_CROSS_COMPILER'

-- Lua source files and include path
local lua_files = [[lapi.c lcode.c ldebug.c ldo.c ldump.c lfunc.c lgc.c llex.c lmem.c lobject.c lopcodes.c
   lparser.c lstate.c lstring.c ltable.c ltm.c lundump.c lvm.c lzio.c lauxlib.c lbaselib.c
   ldblib.c liolib.c lmathlib.c loslib.c ltablib.c lstrlib.c loadlib.c linit.c luac.c print.c lrotable.c]]
lua_files = lua_files:gsub( "\n", "" )
local lua_full_files = utils.prepend_path( lua_files, "src/lua" )
local local_include = "src/lua inc/desktop inc"

local p = c.program{ output .. ".temp", src = lua_full_files, needs = "math", incdir = local_include, defines = cdefs, odir = ".build/cross-lua", flags = "-m32", libflags = "-m32" }
default( utils.build_helper( output, p ) )

