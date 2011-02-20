local args = { ... }
local b = require "utils.build"
local builder = b.new_builder( ".build/rfs_server" )
local utils = b.utils
builder:init( args )
builder:set_build_mode( builder.BUILD_DIR_LINEARIZED )

sim = builder:get_arg( 'sim' )
sim = sim and tonumber( sim ) or 0

local flist, socklib
local cdefs = "RFS_STANDALONE_MODE"
local mainname = sim == 0 and 'main.c' or 'main_sim.c'
local exeprefix = ""
if utils.is_windows() then
  if sim == 1 then
    print "SIM target not supported under Windows"
    os.exit( 1 )
  end
  flist = "main.c server.c os_io_win32.c log.c net_win32.c serial_win32.c deskutils.c"
  cdefs = cdefs .. " WIN32_BUILD"
  exeprefix = "exe"
  socklib = 'ws2_32'
else
  flist = mainname .. " server.c os_io_posix.c log.c net_posix.c serial_posix.c deskutils.c"
end

local output = sim == 0 and 'rfs_server' or 'rfs_sim_server'
local local_include = "rfs_server_src inc/remotefs inc"
local full_files = utils.prepend_path( flist, 'rfs_server_src' ) .. " src/remotefs/remotefs.c src/eluarpc.c"
local compcmd = builder:compile_cmd{ flags = "-m32 -O0 -Wall -g", defines = cdefs, includes = local_include }
local depcmd =  builder:dep_cmd{ flags = "-m32 -O0 -Wall -g", defines = cdefs, includes = local_include }
local linkcmd = builder:link_cmd{ flags = "-m32", libraries = socklib }
builder:set_dep_cmd( depcmd )
builder:set_compile_cmd( compcmd )
builder:set_link_cmd( linkcmd )
builder:set_exe_extension( exeprefix )

-- Build everything
builder:make_depends( full_files )
local odeps = builder:create_compile_targets( full_files )
builder:build_c_target( "rfs_server", odeps )

