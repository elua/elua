import os, sys, platform

output = 'luarpc'
cdefs = ['-DLUA_CROSS_COMPILER','-DLUA_RPC']

# Lua source files and include path
lua_files = """lapi.c lcode.c ldebug.c ldo.c ldump.c lfunc.c lgc.c llex.c lmem.c lobject.c lopcodes.c
   lparser.c lstate.c lstring.c ltable.c ltm.c lundump.c lvm.c lzio.c lauxlib.c lbaselib.c
   ldblib.c liolib.c lmathlib.c loslib.c ltablib.c lstrlib.c loadlib.c linit.c lua.c print.c lrotable.c"""
lua_full_files = " " + " ".join( [ "src/lua/%s" % name for name in lua_files.split() ] )
lua_full_files += " src/modules/luarpc.c src/luarpc_desktop_serial.c "

external_libs = ['m']

if platform.system() == "Windows":
  lua_full_files += " src/serial/serial_win32.c"
  cdefs.append("-DWIN32_BUILD")
else:
  lua_full_files += " src/serial/serial_posix.c"
  external_libs += ['readline']
  cdefs.append("-DLUA_USE_READLINE")

local_include = ['src/lua', 'inc', 'src/modules', 'inc/desktop'];

# Env for building the program
comp = Environment( CPPPATH = local_include,
                    CCFLAGS = cdefs,
                    ENV = os.environ )
# Debug
comp.PrependUnique(CCFLAGS=['-g'])

Decider( 'MD5-timestamp' )                  
Default( comp.Program( output, Split( lua_full_files ), LIBS=external_libs ) )
