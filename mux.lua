local args = { ... }
local b = require "utils.build"
local builder = b.new_builder( ".build/mux" )
local utils = b.utils
builder:init( args )
builder:set_build_mode( builder.BUILD_DIR_LINEARIZED )

local flist = "main.c"
local rfs_flist = "main.c server.c log.c deskutils.c"
local cdefs = "RFS_UDP_TRANSPORT RFS_INSIDE_MUX_MODE"
local socklib
if utils.is_windows() then
  cdefs = cdefs .. " WIN32_BUILD"
  rfs_flist = rfs_flist .. " os_io_win32.c serial_win32.c net_win32.c"
  exeprefix = "exe"
  socklib = "ws2_32"
else
  rfs_flist = rfs_flist .. " os_io_posix.c serial_posix.c net_posix.c"
  exeprefix = ""
end

local full_files = utils.prepend_path( flist, "mux_src" ) .. utils.prepend_path( rfs_flist, "rfs_server_src" ) .. "src/remotefs/remotefs.c src/eluarpc.c"
local local_include = "mux_src rfs_server_src inc inc/remotefs"
local compcmd = builder:compile_cmd{ flags = "-m32 -O0 -Wall -g", defines = cdefs, includes = local_include }
local depcmd =  builder:dep_cmd{ flags = "-m32 -O0 -Wall -g", defines = cdefs, includes = local_include }
local linkcmd = builder:link_cmd{ flags = "-m32", libraries = socklib }
builder:set_dep_cmd( depcmd )
builder:set_compile_cmd( compcmd )
builder:set_link_cmd( linkcmd )
builder:set_exe_extension( exeprefix )

-- Build everyting
builder:make_depends( full_files )
local odeps = builder:create_compile_targets( full_files )
builder:build_c_target( "mux", odeps )

