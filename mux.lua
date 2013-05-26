local utils = require "utils.utils"
local sf = string.format

local flist = "muxmain.c"
local rfs_flist = "main.c server.c log.c deskutils.c rfs_transports.c"
local cdefs = "RFS_UDP_TRANSPORT RFS_INSIDE_MUX_MODE"
if utils.is_windows() then
  cdefs = cdefs .. " WIN32_BUILD"
  rfs_flist = rfs_flist .. " os_io_win32.c serial_win32.c net_win32.c"
else
  rfs_flist = rfs_flist .. " os_io_posix.c serial_posix.c net_posix.c"
end

local full_files = utils.prepend_path( flist, "mux_src" ) .. utils.prepend_path( rfs_flist, "rfs_server_src" ) .. "src/remotefs/remotefs.c src/eluarpc.c"
local local_include = "mux_src rfs_server_src inc inc/remotefs"
local output = "mux"

local p = c.program{ output .. ".temp", src = full_files, needs = "math", incdir = local_include, defines = cdefs, odir = ".build/mux", flags = "-m32", libflags = "-m32", debug = true }
default( utils.build_helper( output, p ) )

