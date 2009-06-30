import os, sys 

output = 'lua'
cdefs = '-DLUA_CROSS_COMPILER -DLUA_REMOTE -DLUA_USE_READLINE'

# Lua source files and include path
lua_files = """lapi.c lcode.c ldebug.c ldo.c ldump.c lfunc.c lgc.c llex.c lmem.c lobject.c lopcodes.c
   lparser.c lstate.c lstring.c ltable.c ltm.c lundump.c lvm.c lzio.c lauxlib.c lbaselib.c
   ldblib.c liolib.c lmathlib.c loslib.c ltablib.c lstrlib.c loadlib.c linit.c lua.c print.c lrotable.c"""
lua_full_files = " " + " ".join( [ "src/lua/%s" % name for name in lua_files.split() ] )
lua_full_files += " src/modules/luarpc.c src/luarpc_posix_serial.c "
local_include = "-Isrc/lua -Iinc -Isrc/modules"

# Compiler/linker options
cccom = "gcc -g %s -Wall %s -c $SOURCE -o $TARGET" % ( local_include, cdefs )
linkcom = "gcc -o $TARGET $SOURCES -lm -lreadline"

# Env for building the program
comp = Environment( CCCOM = cccom,
                    LINKCOM = linkcom,
                    ENV = os.environ )
Decider( 'MD5' )                  
Default( comp.Program( output, Split( lua_full_files ) ) )
