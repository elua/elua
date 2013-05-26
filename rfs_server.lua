local args = { ... }
local b = require "utils.build"
local builder = b.new_builder( ".build/rfs_server" )
local utils = b.utils

-- Set builder options BEFORE calling builder:init
builder:add_option( 'sim', 'run under the eLua simulator', false )
builder:init( arg )

local sim = builder:get_option( 'sim' )
print( sim )

local flist
local cdefs = "RFS_STANDALONE_MODE"
local mainname = sim and 'main_sim.c' or 'main.c'
if utils.is_windows() then
  if sim then
    print "SIM target not supported under Windows"
    os.exit( 1 )
  end
  flist = "main.c server.c os_io_win32.c log.c net_win32.c serial_win32.c deskutils.c rfs_transports.c"
  cdefs = cdefs .. " WIN32_BUILD"
else
  flist = mainname .. " server.c os_io_posix.c log.c net_posix.c serial_posix.c deskutils.c rfs_transports.c"
end

local output = sim and 'rfs_sim_server' or 'rfs_server'
local local_include = "rfs_server_src inc/remotefs inc"
local full_files = utils.prepend_path( flist, 'rfs_server_src' ) .. " src/remotefs/remotefs.c src/eluarpc.c"

local p = c.program{ output .. ".temp", src = full_files, needs = { "math", "sockets" }, incdir = local_include, defines = cdefs, odir = ".build/rfs_server", flags = "-m32", libflags = "-m32", debug = true }
default( utils.build_helper( output, p ) )

